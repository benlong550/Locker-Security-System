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

unsigned char uVal;
unsigned char left = 0x01;
unsigned char right = 0x02;
unsigned char up = 0x03;
unsigned char down = 0x04;
unsigned char select;
unsigned char cnt2 = 0;
void readSelect()
{
	select = GetBit(~PINB, 0);
}
void readInput()
{
	readSelect();
	if (USART_HasReceived(0))
	{
		uVal = USART_Receive(0);
	}
	delay_ms(100);
}

unsigned char mtr[] = {0x01, 0x03, 0x02, 0x06, 0x04, 0x0C, 0x08, 0x09};
unsigned char adm[] = {0x01, 0x01, 0x01, 0x02, 0x02};
int cd;
int bad;
unsigned char free1 = 1;
unsigned char free2 = 1;
unsigned char free3 = 1;
unsigned char arr1[5];
unsigned char arr2[5];
unsigned char arr3[5];
unsigned char arr4[5];
unsigned char cnt;
enum States{Init, wait, UnlockLocker, NewLocker, Admin, L1, L2, L3, loc1, loc2, loc3, newPW1, confirmPW1, EnterPW1, Check1, open1, close1, pw} state;

void UnLocker()
{
	readInput();
	readSelect();
	switch(state)
	{
		case Init:
		uVal = 0;
		state = wait;
		break;
		
		case wait:
		if(uVal == left){uVal = 0;state = UnlockLocker;}
		if(uVal == right){uVal = 0;state = NewLocker;}
		if(uVal == up){uVal = 0;state = Admin;}
		break;
		
		case Admin:
		if(uVal == left){uVal = 0;state = UnlockLocker;}
		if(uVal == right){uVal = 0;state = NewLocker;}
		if(uVal == up){uVal = 0;state = Admin;}
		if(select){state = pw;}
		break;
		
		case pw:
		cnt2 = 0;
		if(cnt >= 4)
		{
			for(int i = 0; i < 5; i++)
			{
				if(arr4[i] != adm[i])
				{
					//LCD_DisplayString(1, "Incorrect adm pw"); LCD_Cursor(0); delay_ms(3000);
					cnt = 0;
					state = wait;
				}
				else{}
			}
			LCD_DisplayString(1, "Correct adm pw"); LCD_Cursor(0); delay_ms(3000);
			state = open1;
		}
		
		break;
		
		case NewLocker:
		readSelect();
		if(select){uVal = 0;state = loc1;}
		if(uVal == left){uVal = 0;state = UnlockLocker;}
		if(uVal == right){uVal = 0;state = NewLocker;}
		if(uVal == up){uVal = 0;state = Admin;}
		break;
		
		case loc1:
		readSelect();
		if(uVal == right){uVal = 0;state = loc2;}delay_ms(400);
		if(free1){if(select){cnt = 0; uVal = 0;state = newPW1;}}
		break;
		
		case loc2:
		if(uVal == right){uVal = 0;state = loc3;}delay_ms(400);
		if(uVal == left){uVal = 0;state = loc1;}delay_ms(400);
		break;
		
		case loc3:
		if(uVal == left){uVal = 0;state = loc2;}delay_ms(400);
		break;
		
		case newPW1:
		readInput();
		if(cnt >= 4){uVal = 0; cnt2 = 0; state = confirmPW1;}
		else{ state = newPW1;}
	
		break;
		
		case confirmPW1:
		readInput();
		if(cnt2 >= 4)
		{
			for(int i = 0; i < 5; i++)
			{
				if(arr4[i] != arr1[i])
				{
					//LCD_DisplayString(1, "Incorrect PW"); LCD_Cursor(0); delay_ms(3000);
					uVal = 0;
					cnt2 = 0;
					state = wait;
				}
			}
			LCD_DisplayString(1, "PW Confirmed!"); LCD_Cursor(0); delay_ms(3000);
			free1 = 0;
			cnt2 = 0;
			uVal = 0;
			state = wait;
		}
		else{uVal = 0; state = confirmPW1;}
		break;
		
		
		case UnlockLocker:
		readInput();
		if(select && !free1){uVal = 0;state = L1;}
		if(uVal == left){uVal = 0;state = UnlockLocker;}
		if(uVal == right){uVal = 0;state = NewLocker;}
		if(uVal == up){uVal = 0;state = Admin;}
		break;
		
		case L1:
		readInput();
		readSelect();
		LCD_DisplayString(1, "Locker 1"); LCD_Cursor(0); delay_ms(400);
		if(uVal == right){uVal = 0;state = L2;}delay_ms(400);
		if(select){state = EnterPW1;}
		break;
		
		case L2:
		
		if(uVal == right){uVal = 0;state = L3;}delay_ms(400);
		if(uVal == left){uVal = 0;state = L1;}delay_ms(400);
		break;
		
		case L3:
		if(uVal == left){uVal = 0;state = L2;}delay_ms(400);
		break;
		
		case EnterPW1:
		readInput();
		if(cnt2 < 5){state = EnterPW1;}
		else{cnt2 = 0; state = Check1;}
		break;
		
		case Check1:
		for(int i = 0; i < 5; i++)
		{
			if(arr4[i] != arr1[i])
			{
				bad = 1;
				//LCD_DisplayString(1, "PW Incorrect"); LCD_Cursor(0); delay_ms(3000);
				state = wait;
				break;
			}
		}
		bad = 0;
		LCD_DisplayString(1, "PW Correct"); LCD_Cursor(0); delay_ms(1000);
		//open locker here
		LCD_DisplayString(1, "Locker 1 Open"); LCD_Cursor(0); delay_ms(1000);
		state = open1;
		break;
		
		case open1:
		state = close1;
		break;
		
		case close1:
		break;
	}

	switch(state)
	{
		case Init:
		break;
		
		case wait:
		LCD_DisplayString(1, "    <-Menu->");
		LCD_Cursor(0);
		delay_ms(100);
		break;
		
		case Admin:
		LCD_DisplayString(1, "Admin"); LCD_Cursor(0);delay_ms(100);
		break;
		
		case pw:
		readInput();
		LCD_DisplayString(1, "Admin PW(5)         *****"); LCD_Cursor(0);delay_ms(1000);
		if(uVal == left){arr1[cnt] = left;cnt++; delay_ms(100);}
		else if(uVal == right){arr1[cnt] = right;cnt++;delay_ms(100);}
		else if(uVal == up){arr1[cnt] = up; cnt++;delay_ms(100);}
		else if(uVal == down){arr1[cnt] = down;cnt++;delay_ms(100);}
		uVal = 0;
		break;
		
		case NewLocker:
		LCD_DisplayString(1, "New Locker"); LCD_Cursor(0);delay_ms(100);
		break;
		
		case loc1:
		LCD_DisplayString(1, "Locker 1");LCD_Cursor(0);delay_ms(100);uVal = 0;
		break;
		
		case loc2:
		LCD_DisplayString(1, "Locker 2");LCD_Cursor(0);delay_ms(100);uVal = 0;
		break;
		
		case loc3:
		LCD_DisplayString(1, "Locker 3");LCD_Cursor(0);delay_ms(100);uVal = 0;
		break;
		
		case newPW1:
		readInput();
		LCD_DisplayString(1, "Create PW(5)         *****"); LCD_Cursor(0);delay_ms(1000); 
		if(uVal == left){arr1[cnt] = left;cnt++; delay_ms(100);}
		else if(uVal == right){arr1[cnt] = right;cnt++;delay_ms(100);}
		else if(uVal == up){arr1[cnt] = up; cnt++;delay_ms(100);}
		else if(uVal == down){arr1[cnt] = down;cnt++;delay_ms(100);}
		uVal = 0;
		break;
		
		case confirmPW1:
		LCD_DisplayString(1, "Reconfirm PW(5)");LCD_Cursor(0); delay_ms(1000);
		readInput();
		if(uVal == left){arr4[cnt2] = left; cnt2++;delay_ms(100);}
		else if(uVal == right){arr4[cnt2] = right; cnt2++;delay_ms(100);}
		else if(uVal == up){arr4[cnt2] = up; cnt2++;delay_ms(100);}
		else if(uVal == down){arr4[cnt2] = down; cnt2++;delay_ms(100);}
		uVal = 0;
		break;
		
		case UnlockLocker:
		LCD_DisplayString(1, "Unlock Locker"); LCD_Cursor(0);delay_ms(100);
		break;
		
		case L1:
		LCD_DisplayString(1, "Locker 1");LCD_Cursor(0);delay_ms(100);
		break;
		
		case L2:
		LCD_DisplayString(1, "Locker 2");LCD_Cursor(0);delay_ms(100);
		break;
		
		case L3:
		LCD_DisplayString(1, "Locker 3");LCD_Cursor(0);delay_ms(100);
		break;
		
		case EnterPW1:
		readInput();
		LCD_DisplayString(1, "Enter PW(5)");LCD_Cursor(0); delay_ms(1000);
		readInput();
		if(uVal == left){arr4[cnt2] = left; cnt2++;delay_ms(700);}
		else if(uVal == right){arr4[cnt2] = right; cnt2++;delay_ms(700);}
		else if(uVal == up){arr4[cnt2] = up; cnt2++;delay_ms(700);}
		else if(uVal == down){arr4[cnt2] = down; cnt2++;delay_ms(700);}
		uVal = 0;
		break;
		
		case Check1:
		if(bad == 1)
		{LCD_DisplayString(1, "PW Incorrect"); LCD_Cursor(0); delay_ms(3000);}
		
		break;
		
		case open1:
		cd = 1024;
		while(cd != 0)
		{
			PORTA = mtr[cnt2];
			delay_ms(3);
			cnt2++;
			if(cnt2 > 7){cnt2 = 0;}
			cd--;
		}
		state = close1;
		break;
		
		case close1:
		if(select)
		{
			cd = 3072;
			while(cd != 0)
			{
				PORTA = mtr[cnt2];
				delay_ms(3);
				cnt2++;
				if(cnt2 > 7){cnt2 = 0;}
				cd--;
			}
			free1 = 1;
			state = wait;
		}
		break;
	}
	
	
		
	
}

int main(void)
{
	DDRA = 0xFF; PORTA = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	PORTB = 0xFF; DDRB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	
	TimerSet(1);
	TimerOn();
	initUSART(0);
	USART_Flush(0);
	
	LCD_init();
	//LCD_ClearScreen();

	PORTB = 0x01;
	//LCD_DisplayString(1, "Ptrn: 1  Spd: 1 uC: 2");.i
    /* Replace with your application code */
	/*
	cd = 1024;
	while(cd != 0)
	{
		PORTA = mtr[cnt2];
		delay_ms(3);
		cnt2++;
		if(cnt2 > 7){cnt2 = 0;}
		cd--;
	} */
	unsigned char tmp;
    while (1) 
    {	
		UnLocker();
    }
}

