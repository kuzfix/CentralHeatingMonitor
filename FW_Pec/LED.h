/*
 * LED.h
 *
 * Created: 29/01/2020 18:52:53
 *  Author: maticpi
 */ 

#ifndef LED_H_
#define LED_H_

#include <avr/io.h>

#define PORT(x) PORT##x
#define DDR(x) DDR##x
#define PIN(x) PIN##x
#define BV(x) (1<<x)
#define STR(x)  #x

#define LED_functions(x,y,n) \
  void L##x##_Init(void) {PORT(y)&=~BV(n); DDR(y)|=BV(n);}\
  void L##x##_On(void) {PORT(y)|=BV(n);}\
  void L##x##_Off(void) {PORT(y)&=~BV(n);}\
  void L##x##_Tgl(void) {PIN(y)=BV(n);}
    
#define LED_declarations(x) \
  void L##x##_Init(void); \
  void L##x##_On(void); \
  void L##x##_Off(void); \
  void L##x##_Tgl(void);
  
LED_declarations(0)
LED_declarations(1)
LED_declarations(2)
LED_declarations(3)
LED_declarations(4)
LED_declarations(5)
LED_declarations(6)
LED_declarations(7)

void LED_Init();

#endif /* LED_H_ */