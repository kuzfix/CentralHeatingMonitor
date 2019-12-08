/*
 * kbd.c
 *
 * Created: 5.6.2014 16:45:06
 *  Author: maticpi
 */ 

#include "kbd.h"

char lastkey;
char lastreleased;

void KBD_Init()
{
	PORTD |= 0x3C;
	DDRD &= 0xC3;
}

char KBD_isKeyStatePressed(char key)
{
	return !(PIND & (1<<(key+1)));
}

char KBD_isKeyStateReleased(char key)
{
	return (PIND & (1<<(key+1)));
}

void KBD_Read()
{
	static char oldD;						//holds the old value of the keyboard IO port
	char newD=(PIND>>2) & 0x0F;				//get the new value of the IO port (keys are connected to PD2, PD3, PD4 and PD5)
	char pressed = (newD ^ oldD) & oldD;	//if the port state has changed, and the old value was 1, the key was pressed
	char released = (newD ^ oldD) & newD;	//if the port state has changed, and the new value is 1, the key was released
	
	if (pressed & 0x01) lastkey=1;			//if the corresponding bit in variable "pressed" is one, then that key was pressed
	if (pressed & 0x02) lastkey=2;			// (lastkey can only hold one value, therefore if more than one key was pressed 
	if (pressed & 0x04) lastkey=3;			// at once, one event will be lost)
	if (pressed & 0x08) lastkey=4;
	
	if (released & 0x01) lastreleased=1;			//if the corresponding bit in variable "released" is one, then that key was pressed
	if (released & 0x02) lastreleased=2;			// (lastreleased can only hold one value, therefore if more than one key was released
	if (released & 0x04) lastreleased=3;			// at once, one event will be lost)
	if (released & 0x08) lastreleased=4;
	
	oldD=newD;								//update the 
}

char KBD_GetKey()
{
	char tmp=lastkey;
	lastkey=0;
	return tmp;
}

char KBD_GetReleasedKey()
{
	char tmp=lastreleased;
	lastreleased=0;
	return tmp;
}

void KBD_flush()
{
	lastkey=0;
	lastreleased=0;
}
