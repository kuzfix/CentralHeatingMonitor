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
	BTN1_PORT |= (1<<BTN1_BIT);
	BTN2_PORT |= (1<<BTN2_BIT);
	BTN3_PORT |= (1<<BTN3_BIT);
#if 0
	BTN4_PORT |= (1<<BTN4_BIT);
	BTN6_PORT |= (1<<BTN6_BIT);
	BTN7_PORT |= (1<<BTN7_BIT);
#endif

	BTN1_DDR &=~ (1<<BTN1_BIT);
	BTN2_DDR &=~ (1<<BTN2_BIT);
	BTN3_DDR &=~ (1<<BTN3_BIT);
#if 0
	BTN4_DDR &=~ (1<<BTN4_BIT);
	BTN6_DDR &=~ (1<<BTN6_BIT);
	BTN7_DDR &=~ (1<<BTN7_BIT);
#endif
}

char KBD_isKeyStatePressed(char key)
{
	char result=0;
	switch (key)
	{
		case BTN1: result = (BTN1_PIN & (1<<BTN1_BIT)) != (1<<BTN1_BIT); break;
		case BTN2: result = (BTN2_PIN & (1<<BTN2_BIT)) != (1<<BTN2_BIT); break;
		case BTN3: result = (BTN3_PIN & (1<<BTN3_BIT)) != (1<<BTN3_BIT); break;
#if 0
		case BTN4: result = (BTN4_PIN & (1<<BTN4_BIT)) != (1<<BTN4_BIT); break;
		case BTN6: result = (BTN6_PIN & (1<<BTN6_BIT)) != (1<<BTN6_BIT); break;
		case BTN7: result = (BTN7_PIN & (1<<BTN7_BIT)) != (1<<BTN7_BIT); break;
#endif
	}
	return result;
}

char KBD_isKeyStateReleased(char key)
{
	char result=0;
	switch (key)
	{
		case BTN1: result = (BTN1_PIN & (1<<BTN1_BIT)) == (1<<BTN1_BIT); break;
		case BTN2: result = (BTN2_PIN & (1<<BTN2_BIT)) == (1<<BTN2_BIT); break;
		case BTN3: result = (BTN3_PIN & (1<<BTN3_BIT)) == (1<<BTN3_BIT); break;
#if 0
		case BTN4: result = (BTN4_PIN & (1<<BTN4_BIT)) == (1<<BTN4_BIT); break;
		case BTN6: result = (BTN6_PIN & (1<<BTN6_BIT)) == (1<<BTN6_BIT); break;
		case BTN7: result = (BTN7_PIN & (1<<BTN7_BIT)) == (1<<BTN7_BIT); break;
#endif
	}
	return result;
}

void KBD_Read()
{
	static char oldState;						//holds the old value of the keyboard IO port
	char newState;
	char pressed;
	char released;

	//get the new value of the IO port
	newState = 0;
	newState |= KBD_isKeyStatePressed(BTN1)<<0;
	newState |= KBD_isKeyStatePressed(BTN2)<<1;
	newState |= KBD_isKeyStatePressed(BTN3)<<2;
#if 0
	newState |= KBD_isKeyStatePressed(BTN4)<<3;
	newState |= KBD_isKeyStatePressed(BTN6)<<4;
	newState |= KBD_isKeyStatePressed(BTN7)<<5;
#endif
	pressed = (newState ^ oldState) & oldState;	//if the port state has changed, and the old value was 1, the key was pressed
	released = (newState ^ oldState) & newState;	//if the port state has changed, and the new value is 1, the key was released
	
	if (pressed & (1<<0)) lastkey=BTN1;			//if the corresponding bit in variable "pressed" is one, then that key was pressed
	if (pressed & (1<<1)) lastkey=BTN2;			// (lastkey can only hold one value, therefore if more than one key was pressed 
	if (pressed & (1<<2)) lastkey=BTN3;			// at once, one event will be lost)
#if 0
	if (pressed & (1<<3)) lastkey=BTN4;
	if (pressed & (1<<4)) lastkey=BTN6;
	if (pressed & (1<<5)) lastkey=BTN7;
#endif
	
	if (released & (1<<0)) lastreleased=BTN1;			//if the corresponding bit in variable "released" is one, then that key was pressed
	if (released & (1<<1)) lastreleased=BTN2;			// (lastreleased can only hold one value, therefore if more than one key was released
	if (released & (1<<2)) lastreleased=BTN3;			// at once, one event will be lost)
#if 0
	if (released & (1<<3)) lastreleased=BTN4;
	if (released & (1<<4)) lastreleased=BTN6;
	if (released & (1<<5)) lastreleased=BTN7;
#endif
  
	oldState=newState;								//update the 
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
