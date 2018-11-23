/*
 * EN0077_Lync.c
 * Version 1.0
 * For ATtiny 9 --- but 4,5 and 10 should work too
 *
 * Created: 20.09.2014 14:39:09
 *  Author: Jarno Thierolf, Horst Platz Beratungs- & Vertriebs GmbH
 */ 

#ifndef F_CPU
#define F_CPU    1000000UL
#warning "Variable F_CPU wasn't configured before, it will be set to 1MHz"
#endif

#define lync PINB0		//set impulse here for toggle lync hook state
#define call PINB1		//0=no call; 1=call active
#define hook PINB2		//0=hook on; 1=hook off

#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

uint8_t cliFlag = 0;

int main(void)
{
	DDRB = 0x01;			//PB0-lync output, PB1-call + PB2-hook are inputs 0b00000001
	PORTB = 0x00;			//all pull-ups off
	
	EIMSK = (1 << INT0);	//activate external interrupt INT0 (if I-bit is set)
	EICRA = (1 << ISC00);	//Any logical change on INT0 generates an interrupt request.
	
    while(1)
    {
		if (cliFlag == 0)
		{
			sei ();					// set I-bit (activate external interrupts in general)
			
			set_sleep_mode (SLEEP_MODE_PWR_DOWN);
			sleep_mode();			//Standby until external interrupt at input PB2
		}
		else
		{
			cli();					// del I-bit (deactivate external interrupts in general)
			
			uint8_t callState = (PINB & (1 << call));
			uint8_t hookState = (PINB & (1 << hook));

			if(((!callState) && hookState) || (callState && (!hookState)))	//callState XOR hookState
			{
				PORTB |= (1 << lync);		//set PB0 high
				_delay_ms(100);				//impulse and debouncing time
				PORTB &= ~(1 << lync);		//set PB0 low
			}
			else
			{
				//do nothing
			}
			
			cliFlag = 0;					//reset -> Interrupts on and sleep
		}
    }
}

ISR(INT0_vect)		//extern interrupt for PB2 on every hook change
{
	cliFlag = 1;	//set cliFlag to start impulse and for avoiding bouncing at input
}