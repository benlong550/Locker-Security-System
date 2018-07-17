/*
 * Demo1.c
 *
 * Created: 11/29/2017 11:22:14 PM
 * Author : Benjamin
 */ 

#include <avr/io.h>
//#include "io.c"
//#include "io.h"
#include "lcd.h"
#include "bit.h"
#include <avr/interrupt.h>
#include "usart_ATmega1284.h"

//timer code
volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.

// Internal variables for mapping AVR's ISR to our cleaner Timer ISR model.
unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn() {
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;
	TCNT1 = 0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}

void TimerOff() {
	TCCR1B = 0x00;
}

void TimerISR() {
	TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect) {
	_avr_timer_cntcurr--;
	if (_avr_timer_cntcurr == 0) {
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}


//JOYSTICK CODE
void A2D_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
	// ADEN: Enables analog-to-digital conversion
	// ADSC: Starts analog-to-digital conversion
	// ADATE: Enables auto-triggering, allowing for constant
	// analog to digital conversions.
}
// Pins on PORTA are used as input for A2D conversion
// The default channel is 0 (PA0)
// The value of pinNum determines the pin on PORTA
// used for A2D conversion
// Valid values range between 0 and 7, where the value
// represents the desired pin for A2D conversion
void Set_A2D_Pin(unsigned char pinNum) {
	ADMUX = (pinNum <= 0x07) ? pinNum : ADMUX;
	// Allow channel to stabilize
	static unsigned char i = 0;
	for ( i=0; i<15; i++ ) { asm("nop"); }
}
// Sets PA0 to be input pin for A2D conversion
//Set_A2D_Pin(0x00);
	unsigned short lr;
	unsigned short ud;
void readyjoystick()
{
	A2D_init();
	
	Set_A2D_Pin(0x00);
	lr = ADC;
	Set_A2D_Pin(0x01);
	ud = ADC;
	delay_ms(500);
}
//STATE MACHINES
unsigned char uVal;
unsigned char left = 0x01;
unsigned char right = 0x02;
unsigned char up = 0x03;
unsigned char down = 0x04;


enum States{Init, Wait, UP, DOWN, LEFT, RIGHT} state;

unsigned char tmpB = 0x00;
unsigned char tmpA = 0x00;
int cnt = 0;
int cnt2 = 0;


void ActivateJoystick()
{
	readyjoystick();
	switch(state)
	{
		case Init:
		state = Wait;
		break;

		case Wait:
		if(lr > 400 && lr < 700 && ud > 400 && ud < 700){state = Wait;}
		if(lr > 800){state = RIGHT;}
		if(lr < 200){state  = LEFT;}
		if(ud > 800){state = UP;}
		if(ud < 200){state = DOWN;}
		break;
		
		case UP:
		if(ud > 800){state = UP;}
		else{state = Wait;}
		break;
		
		case DOWN:
		if(ud < 200){state = DOWN;}
		else{state = Wait;}
		break;
		
		case LEFT:
		if(lr < 200){state  = LEFT;}
		else{state = Wait;}
		break;
		
		case RIGHT:
		if(lr > 800){state = RIGHT;}
		else{state = Wait;}
		break;
	}

	switch(state)
	{
		case Init:
		break;

		case Wait:
		break;
		
		case LEFT: PORTB = 0x01;
		if (USART_IsSendReady(0)){
			USART_Send(left, 0);
		}
		if (USART_HasTransmitted(0))
		{
			USART_Flush(0);
			delay_ms(100);
		}
		break;
		
		case RIGHT: PORTB = 0x02;
		if (USART_IsSendReady(0)){
			USART_Send(right, 0);
		}
		if (USART_HasTransmitted(0))
		{
			USART_Flush(0);
			delay_ms(100);
		}
		break;
		
		case UP: PORTB = 0x04;
		if (USART_IsSendReady(0)){
			USART_Send(up, 0);
		}
		if (USART_HasTransmitted(0))
		{
			USART_Flush(0);
			delay_ms(100);
		}
		break;
		
		case DOWN: PORTB = 0x08;
		if (USART_IsSendReady(0)){
			USART_Send(down, 0);
		}
		if (USART_HasTransmitted(0))
		{
			USART_Flush(0);
			delay_ms(100);
		}
		break;
	}
	
}

int main(void)
{
	DDRD = 0xFF; PORTD = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	
	TimerSet(1);
	TimerOn();
	
	A2D_init();
	initUSART(0);
	USART_Flush(0);
	
/*uVal = 0x01;
if (USART_IsSendReady(0)){
	USART_Send(uVal, 0);
}
if (USART_HasTransmitted(0))
{
	USART_Flush(0);
	delay_ms(5000);
}*/

    while (1) 
    {		
		ActivateJoystick();
	}
}

