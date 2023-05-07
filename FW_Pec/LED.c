/*
 * LED.c
 *
 * Created: 29/01/2020 18:52:31
 *  Author: maticpi
 */ 

#include "LED.h"

LED_functions(0,D,4)
LED_functions(1,E,1)
LED_functions(2,C,3)
LED_functions(3,C,2)
LED_functions(4,E,2)
LED_functions(5,C,1)
LED_functions(6,C,0)
LED_functions(7,E,3)

char txt[]=STR(LED_functions(0,D,4));

void LED_Init()
{
  L0_Init();
  L1_Init();
  L2_Init();
  L3_Init();
  L4_Init();
  L5_Init();
  L6_Init();
  L7_Init();
}