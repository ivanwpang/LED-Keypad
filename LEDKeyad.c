// RedID: 822740276
// X = 2
// Y = 7
// Z = 6
// X + 1 = 2 + 1 = 3
// Interrupt every 3ms
// (Z + 1) * 100Hz = 700Hz
// PWM frequency = 700Hz
// 1 / 700Hz = 0.001429 = 1.429ms = 1ms
// Max Time Value = 1 / 16MHz * 256 = 0.000016s = 16?s = 0.016ms >> too small
// 0.016ms * 256 = 4.096ms 
// Pre-Scale = 256
// (256 * 3ms) / 4.096ms = 187.5 = 188
// OCR0A = 188 - 1 = 187
// Pre-Scale = 64
// 0.016 * 64 = 1.024ms
// (256 * 1ms) / 1.024ms = 250
// OCR2A = 250 - 1 = 249
#define  F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

unsigned short keychar[4][4] = {{1, 2, 3, 0},
								{4, 5, 6, 0},
								{7, 8, 9, 0},
								{0, 0, 0, 0}};

void timer_zero();
void timer_two(float x);
void scan_keypad(unsigned short keychar[4][4]);

int main(void)
{
	DDRB |= (1 << 5);	// Set B5 to output
	 for(int i = 4; i < 8; i++) {                // Set all the ports D4 - D7 to output
		 DDRD |= (1 << i);
	 }
	 for(int i = 0; i < 4; i++) {                // Set all the ports B0 - B3 to input
		 DDRB &= ~(1 << i);
	 }
	PORTB |=(1<<0|1<<1|1<<2|1<<3); //Set High
	PORTD |= (1<<4|1<<5|1<<6|1<<7); //Set High

	// Interrupt Initialization for Timer 0
	timer_zero();
	sei();
		
    while (1) 
    {
    }
}

ISR (TIMER0_COMPA_vect) {
	scan_keypad(keychar);	// Scan keypad every 3ms
}

ISR (TIMER2_COMPA_vect) {
	if (OCR2B > 0) {		// Check if duty cycle is > 0
		PORTB |= (1 << 5);	// Turn on LED
	}
	else {
		PORTB &= ~(1 << 5);		// Turn off LED
	}
}

ISR (TIMER2_COMPB_vect) {
	PORTB &= ~(1 << 5);		// Turn off LED
}

void scan_keypad(unsigned short keychar[4][4]) {
	PORTD |= (1 << 4 | 1 << 5 | 1 << 6 | 1 << 7);    // Set all rows(output) high
	for(int i = 4; i < 8; i++) {
		PORTD &= ~(1 << i);                            // Set row(i) low - one at a time and check each column
		for(int j = 0; j < 4; j++) {
			if (!(PINB & (1 << j))) {                // Check each column(j) to see which is pulled Low
				timer_two(keychar[i-4][j]);        // Output the corresponding character
				//_delay_ms(200);                        // De-bounce time of 200ms
			}
		}
		PORTD = 0xFF;                                // Set all the pins of Port D to 1 (Pull-Up Resistor)
	}
}
// Keypad Timer
void timer_zero() {
	OCR0A = 187;	// Set count to 187
	TCCR0A |= 1 << WGM01;	// Set timer to CTC
	TCCR0B |= (1 << CS22);	// Set pre-scale to 256
	TIMSK0 |= (1 << OCIE0A);	// Enable Compare Match A ISR
}
// LED Timer
void timer_two(float x) {
	OCR2A = 249;	// Set count to 249
	OCR2B = (OCR2A * (x / 10)) ;	 // Set duty cycle	Ex: OCR2A * (9 / 10) = OCR2A * (0.9)
	TCCR2A |= 1 << WGM21;	// Set timer to CTC
	TCCR2B |= (1 << CS00) | (1 << CS01);	// Set pre-scale to 64
	TIMSK2 |= (1 << OCIE2A) | (1 << OCIE2B);	// Enable Compare Match A and B ISR
}
