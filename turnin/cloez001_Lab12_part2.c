
/*      Author: lab
 *  Partner(s) Name: 
 *      Lab Section:
 *      Assignment: Lab #  Exercise #
 *      Exercise Description: [optional - include for your own benefit]
 *
 *      I acknowledge all content contained herein, excluding template or examp$
 *      code, is my own original work.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programme$

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start counter from here to 0. Default 1ms
unsigned long _avr_timer_cntcurr = 0; // Current internal clock of 1ms ticks

void TimerOn(){
    // AVR timer/counter controller register TCCR1
    TCCR1B = 0x0B;      // bit3 = 0: CTC mode(clear timer on compare)
                        // bit2bit1bit0 = 011;
                        // 0000 1011 : 0x0B
                        // So 8 MHz clock or 8,000,000 / 64  = 125,000 ticks
                        // Thus TCNT1 register will count 125,000 ticks

    // AVR out compare register OCR1A
    OCR1A = 125;        // Timer interrupt will be generated when TCNT1==OCR1A
                        // We want a 1ms tick. .001s * 125,000 ticks = 125
                        // So when TCNT1 == 125 then that means 1ms has passed

    // AVR timer register interrupt mask register
    TIMSK1 = 0x02;      // bit1: OCIE1A -- enables compare match register

    // Initiliaze AVR counter
    TCNT1 = 0;

    _avr_timer_cntcurr = _avr_timer_M;

    // enable global interrupts
    SREG |= 0x80;       // 1000 0000 
}

void TimerOff(){
    TCCR1B = 0x00;
}

void TimerISR(){
    TimerFlag = 1;
}

// In our approach C program does not touch this ISR, only TimerISR()
ISR(TIMER1_COMPA_vect){
    // CPU automatically calls when TCNT1 == OCR1 (Every 1ms per TimerOn settin$
    _avr_timer_cntcurr--;       // count down to 0
    if (_avr_timer_cntcurr == 0){
        TimerISR();     // call the ISR that the user uses
        _avr_timer_cntcurr = _avr_timer_M;
    }
}

// Set timer to tick every M ms.
void TimerSet(unsigned long M){
    _avr_timer_M = M;
    _avr_timer_cntcurr =  _avr_timer_M;
}



//--------------------------------------
// LED Matrix Demo SynchSM
// Period: 100 ms
//--------------------------------------
unsigned char set_row;
unsigned char set_pattern;
unsigned char button;
unsigned char btn;

enum Demo_States {shift};
int Demo_Tick(int state) {

    // Local Variables
    static unsigned char pattern = 0x80;	// LED pattern - 0: LED off; 1: LED on
    static unsigned char row = 0x00;  	// Row(s) displaying pattern. 
							// 0: display pattern on row						// 1: do NOT display pattern on row
    // Transitions
    switch (state) {
	case shift:	
	    break;
	default:	
	    state = shift;
	    break;
    }	
    // Actions
    switch (state) {
	case shift:	
/*
	    if (set_row == 0xEF && button == 2) { // Reset demo 		    
		row = 0xEF;
	    } else if (set_row != 0xEF && button == 2) { // Move LED to start of next row
		//row = (row << 1) | 0x01;
		row = set_row ;
	    } else if (row == 0xFE && button == 1){
		row = 0xFE;
	    } else if (row != 0xFE && button == 1){
		row = set_row;
	    } 
*/
	    //pattern = set_pattern;
	    if (pattern == 0x01 && button == 3){
		pattern = 0x01;
	    } else if (pattern != 0x01 && button == 3){
		pattern = set_pattern;
	    } else if (pattern == 0x80 && button == 4){
		pattern = 0x80;
	    } else if (pattern != 0x80 && button == 4){
		pattern = set_pattern;
	    } 
	    break;
	
	default:
	    break;
    }
    PORTC = pattern;	// Pattern to display
    PORTD = row;		// Row(s) displaying pattern	
    return state;

}

//unsigned char set_row;
enum Change_States {Change_Start, Change_Wait, Change_Decrement, Change_Increment, Change_Right, Change_Left} Change_State;

void Calculate(){
    // Transition
    switch(Change_State){
	case Change_Start:
	    Change_State = Change_Wait;
	    button = 0;
	    break;
	case Change_Wait:
	    if(btn == 0){
	 	button = 0;
		Change_State = Change_Wait;
	    }
	    else if (btn == 0x02){
		Change_State = Change_Decrement;
		button = 2;
     	    } else if (btn == 0x01) {
		Change_State = Change_Increment;
		button = 1;
	    } else if(btn == 0x04){
		Change_State = Change_Right;
		button = 3;
	    } else if(btn == 0x08){
		Change_State = Change_Left;
		button = 4;
	    }
	    break;

	case Change_Decrement:
	    Change_State = Change_Wait;
	    break;

	case Change_Increment:
	    Change_State = Change_Wait;
	    break;

	case Change_Right:
	    Change_State = Change_Wait;
	    break;

	case Change_Left:
	    Change_State = Change_Wait;
	    break;

	default:
	    Change_State = Change_Wait;
	    break;
    }

    // Action
    switch(Change_State){
	case Change_Wait:
	    break;
	case Change_Decrement:
	    //if(set_row != 0xEF && button == 2)
		//set_row = (set_row << 1) | 0x01;
	    break;
	case Change_Increment:
	    //if(set_row != 0xFE && button == 1){
	 	//set_row = (set_row >> 1) | 0x80;
	    //}
	    break;

	// Shift column to the right
	case Change_Right:
	    if(set_pattern != 0x01 && button == 3)
		set_pattern = (set_pattern >> 1);
	    break;
	// Shift column to the left
	case Change_Left:
	    if(set_pattern != 0x80 && button == 4)
		set_pattern = (set_pattern << 1);
	    break;

	default:
	    break;
    }
}


int main(void) {
    /* Insert DDR and PORT initializations */
    
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;
    DDRA = 0x00; PORTA = 0x0F;

    set_row = 0x00;
    set_pattern = 0x80;
    Change_State = Change_Start;

    int state = 0;
    TimerSet(100);
    TimerOn();
    /* Insert your solution below */
    while (1) {
	btn = ~PINA & 0x0F;
        Calculate();
	state = Demo_Tick(state);

	while(!TimerFlag);
        TimerFlag = 0;
    }
    return 1;
}



