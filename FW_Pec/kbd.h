/*
 * LCD.h
 *
 * Created: 23.4.2013 22:14:06
 *  Author: maticpi
 */ 

#ifndef KBD_H_
#define KBD_H_

#include <avr/io.h>			//header file with register definitions
/* B1,B2
PD2
PE0
PB1 - LCD TOUCH IRQ
*/

#define BTN1	1
#define BTN2	2
#define BTN3	3

#define BTN1_DDR	DDRD
#define BTN2_DDR	DDRE
#define BTN3_DDR	DDRB

#define BTN1_PORT	PORTD
#define BTN2_PORT	PORTE
#define BTN3_PORT	PORTB

#define BTN1_PIN	PIND
#define BTN2_PIN	PINE
#define BTN3_PIN	PINB

#define BTN1_BIT	2
#define BTN2_BIT	0
#define BTN3_BIT	1

void KBD_Init();
void KBD_Read();
char KBD_GetKey();
char KBD_GetReleasedKey();
void KBD_flush();
char KBD_isKeyStatePressed(char key);
char KBD_isKeyStateReleased(char key);

extern char lastkey;
extern char lastreleased;

#endif /* KBD_H_ */