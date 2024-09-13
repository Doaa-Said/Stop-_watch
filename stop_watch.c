/*
 *
 * stop_watch.c
 *  Created on: Sep 11, 2024
 *  Author: Doaa Said
 *
 */
# include<avr/io.h>
# include<util/delay.h>
# include<avr/interrupt.h>
unsigned char seconds = 0;
unsigned char minutes = 0;
unsigned char hours = 0;
unsigned char flag = 0; //toggle mode button flag
void Timer1_CTC_init(void); //initialize timer1&set clock&mode
void INT0_Init(void); // External Interrupt 0 initialization
void INT1_Init(void); // External Interrupt 1 initialization
void INT2_Init(void); // External Interrupt 2 initialization
void display(void); // Function to update the display
void IO_PORTS_init(void); // Function to initialize I/O ports

int main(void) {
	Timer1_CTC_init();
	INT0_Init();
	INT1_Init();
	INT2_Init();
	IO_PORTS_init();
	SREG |= 1 << 7; //Enable global interrupts

	for (;;) {
		display();
		if ((!(PINB & 1 << 7)) && (flag == 0)) {
			//check if toggle mood button is pressed
			PORTD ^= (1 << 4);
			PORTD ^= (1 << 5);
			flag = 1;
		}
		if (PINB & (1 << 7)) {
			flag = 0;
// button is released reset the toggle mode button flag to value 0 again.
		}
		display(); // Update display
//Alarm
		if ((hours == 0) && (minutes == 0) && (seconds == 0)
&& (PORTD & 1 << 5)&& (TCCR1B & 1 << CS10) && (TCCR1B & (1 << CS12)))
			PORTD |= 1 << 0;
		//turn on buzzer if hrs&mins&secs reach zero in count down mode
		else
			PORTD &= ~(1 << 0);

		display(); // Update display

		if (!(PINB & (1 << PINB6)) && (seconds < 59)) {
			// Check if button connected to PINB6 is pressed
			_delay_ms(30); // Debounce delay
			if (!(PINB & (1 << PINB6)) && (seconds < 59)) {
				seconds++;
			}
			// Wait for button release
			while (!(PINB & (1 << PINB6))) {
				display(); // Update display
			}

		} else if (!(PINB & (1 << PB5)) && (seconds != 0)) {
			// Check if button connected to PB5 is pressed
			_delay_ms(30); // Debounce delay
			if (!(PINB & (1 << PB5)) && (seconds != 0)) {
				seconds--;
				// Wait for button release
				while (!(PINB & (1 << PB5))) {
					display(); // Update display
				}

			}
		}

		if ((!(PINB & 1 << 0)) && hours > 0) {
			// Check if button connected to PINB0 is pressed
			_delay_ms(30); //Debounce delay
			if ((!(PINB & 1 << 0)) && hours > 0)
				hours--;
			// Wait for button release
			while (!(PINB & (1 << 0))) {
				display(); // Update display
			};

		}

		else if ((!(PINB & (1 << 1)))) {
			// Check if button connected to PINB1 is pressed
			_delay_ms(30); //Debounce delay
			if ((!(PINB & (1 << 1)))) {
				hours++;
				// Wait for button release
				while (!(PINB & (1 << 1))) {
					display(); // Update display
				};

			}
		}
		if ((!(PINB & (1 << 3))) && (minutes > 0)) {
			// Check if button connected to PINB3 is pressed
			_delay_ms(30); //Debounce delay
			if ((!(PINB & (1 << 3))) && (minutes > 0)) {
				minutes--;
				// Wait for button release
				while (!(PINB & (1 << 3))) {
					display(); // Update display
				};

			}
		} else if ((!(PINB & (1 << 4))) && minutes < 59) {
			// Check if button connected to PINB4 is pressed
			_delay_ms(30); //Debounce delay
			if ((!(PINB & (1 << 4))) && minutes < 59)
				minutes++;
			// Wait for button release
			while (!(PINB & (1 << 4))) {
				display(); //// Update display
			};

		}

		display(); // Update display

	}
}
void Timer1_CTC_init(void) {
	TCCR1A = 1 << FOC1A; //Normal mode(Not pwm)
	TCCR1B = 1 << WGM12 | 1 << CS10 | 1 << CS12;
	//CTC mode, prescaler = 1024
	TCNT1 = 0; //initial value 0
	OCR1A = 15625; //interrupts every 1sec
	TIMSK = 1 << OCIE1A; //Output Compare A Match Interrupt Enable

}
// Timer1 compare match interrupt service routine
ISR(TIMER1_COMPA_vect) {
	SREG |= 1 << 7;
	if (PORTD & (1 << 4)) { // count up mode(red led indicator)
		if (seconds <= 59)
			seconds++;
		else if (seconds == 60) {
			minutes++;
			seconds = 0;
		} else if (minutes == 60) {
			hours++;
			minutes = 0;
			seconds = 0;

		}
	} else if (PORTD & (1 << 5)) {
		// count down mode(yellow led indicator)

		if (seconds > 0)
			seconds--;
		else if (seconds == 0 && minutes != 0) {
			minutes--;

			seconds = 59;

		} else if (minutes == 0 && hours != 0) {
			hours--;
			minutes = 59;

		}

	}

}
// External Interrupt 0 initialization
void INT0_Init(void) {
	DDRD &= ~(1 << 2); // Set PD2 as input
	PORTD |= 1 << 2; // Enable internal pull-up resistor
	// Trigger on falling edge
	MCUCR |= 1 << ISC01;
	MCUCR &= ~(1 << ISC00);
	GICR |= 1 << INT0;	// Enable INT0

}
// External interrupt 0 service routine
ISR(INT0_vect) {
	seconds = 0;
	minutes = 0;
	hours = 0;
	Timer1_CTC_init();
	GIFR |= 1 << INTF0;

//reset stop watch
}
// External Interrupt 1 initialization
void INT1_Init(void) {
	DDRD &= ~(1 << 3);	// Set PD3 as input
	MCUCR |= 1 << ISC11 | 1 << ISC10;	// Trigger on rising edge
	GICR |= 1 << INT1;	// // Enable INT1
}
// External interrupt 1 service routine
ISR(INT1_vect) {
	
	TCCR1B &= ~(1 << CS10) & ~(1 << CS11) & ~(1 << CS12);
	GIFR |= 1 << INTF1;
//stop Timer1}
//pause stop watch
}
// External Interrupt 2 initialization
void INT2_Init(void) {
	DDRB &= ~(1 << 2);	// Set PB2 as input
	PORTB |= 1 << 2;	// Enable internal pull-up resistor
	MCUCR |= 1 << ISC2; // Trigger on falling edge
	GICR |= 1 << INT2; //// Enable INT2
}
// External interrupt 2 service routine
ISR(INT2_vect) {
	TCCR1B |= 1 << CS10 | 1 << CS12;
	GIFR |= 1 << INTF2;
// resume stop watch
}
// Function to update the display
void display(void) {
	// Display hours
	PORTA &= (0xc1);
	PORTA |= (0x01); // Activate digit for hours tens
	PORTC = (PORTC & 0xf0) | ((hours / 10) & 0x0f);
	_delay_ms(3);
	PORTA &= (0xc2);
	PORTA |= (0x02); // Activate digit for hours units
	PORTC = (PORTC & 0xf0) | ((hours % 10) & 0x0f);
	_delay_ms(3);
	//display minutes
	PORTA &= (0xc4);
	PORTA |= (0x04); // Activate digit for minutes tens
	PORTC = (PORTC & 0xf0) | ((minutes / 10) & 0x0f);
	_delay_ms(3);
	PORTA &= (0xc8);
	PORTA |= (0x08); // Activate digit for minutes units
	PORTC = (PORTC & 0xf0) | ((minutes % 10) & 0x0f);
	_delay_ms(3);
	//display seconds
	PORTA &= 0xD0;
	PORTA |= 0x10; // Activate digit for seconds tens
	PORTC = (PORTC & 0xf0) | ((seconds / 10) & 0x0f);
	_delay_ms(3);
	PORTA &= 0xE0;
	PORTA |= 0x20; // Activate digit for minutes units
	PORTC = (PORTC & 0xf0) | ((seconds % 10) & 0x0f);
	_delay_ms(3);

}

void IO_PORTS_init(void) {
	DDRC |= (0x0f); // Set first four pins of portc as output
	DDRA |= (0X3F); // Set PA0-PA5 as output
	DDRB &= ~(1 << 7); // Set PB7 as input(toggle mode button)
	PORTB |= 1 << 7; //enable internal pull-up resistor
	DDRD |= 1 << 4 | 1 << 5;
	// Set PD4 and PD5 as output(leds indicate mode)
	DDRD |= (1 << 0); //Set PD0 as output(buzzer)
	PORTD &= 1 << 0; //clear PD0 initially
	PORTD |= (1 << 4); //set count up mode as default
	PORTD &= ~(1 << 5); //turn off led that indicates count down mode
	DDRB &= ~(1 << 0) & ~(1 << 1) & ~(1 << 3) & ~(1 << 4) & ~(1 << 5)
			& ~(1 << 6);
//setPB0-PB1-PB3-PB4-PB5-PB6 as input
	PORTB |= 1 << 0 | 1 << 1 | 1 << 3 | 1 << 4 | 1 << 5 | 1 << 6;
	//enable internal pull-up resistor
}

