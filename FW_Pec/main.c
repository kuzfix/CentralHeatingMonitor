/*
 * FW_Pec.c
 *
 * Created: 01-Dec-18 11:25:12
 * Author : maticpi
 */ 

#include <avr/io.h>
#include <string.h>
#include "config.h"
#include "kbd.h"
#include "systime.h"
#include "UART0_IRQ.h"
#include "adc.h"
#include "XPT2046.h"
#include "LCD_Ili9341.h"
#include "MAX31855.h"
#include "nrf24.h"
#include "Graphics.h"
#include "DS18B20management.h"

/* PROTOCOL
ID valH valL Vdd rtr
ID: 7..3 = ID, 2..0 = sensor type
valH - raw value HIGH byte.
valL - raw value LOW byte.
Vdd - power supply voltage
rtr - number of retransmissions of the previous packet (indicates ink quality, lower is better)
*/
#define STYPE_Tds18b20		0x01
#define STYPE_Tsht21			0x02
#define STYPE_Hsht21			0x03
#define STYPE_Tbmp280			0x05
#define STYPE_Pbmp280			0x06

void DecodeCommand(uint8_t *pu8_cmd)
{
  uint8_t   ID = pu8_cmd[0]>>3;
  uint16_t  raw = (pu8_cmd[1]<<8) | pu8_cmd[2];
  float sensor_val=0;
  
  switch (pu8_cmd[0] & 0x07)
  {
    case STYPE_Tds18b20:
    if (raw == 0xFFFF) sensor_val = -99.9;
    if (raw & 0x0800) sensor_val = (double)((int)(raw | 0xf000));
    else sensor_val = raw;
    sensor_val/=16.0;
    printf_P(PSTR(" DS18B20: ID=%d T=%.2fC"), ID, sensor_val);
    break;
    
    case STYPE_Tsht21:
    if (raw == 0xFFFF) sensor_val = -99.9;
    sensor_val = -46.85 + 175.72 / 65536.0 * (float)raw;
    printf_P(PSTR(" SHT21:   ID=%d T=%.2fC"), ID, sensor_val);
    break;
    
    case STYPE_Hsht21:
    if (raw == 0xFFFF) sensor_val = -99.9;
    sensor_val = -6.0 + 125.0 / 65536.0 * (float)raw;
    printf_P(PSTR(" SHT21:   ID=%d H=%.2f%%RH"), ID, sensor_val);
    break;
    
    case STYPE_Tbmp280:
    if (raw == 0xFFFF) sensor_val = -99.9;
    sensor_val = raw;
    printf_P(PSTR(" BMP280:  ID=%d T=%.2fC"), ID, sensor_val);
    break;
    
    case STYPE_Pbmp280:
    if (raw == 0xFFFF) sensor_val = -99.9;
    sensor_val = raw;
    printf_P(PSTR(" BMP280:  ID=%d P=%.2fmBar"), ID, sensor_val);
    break;
    
    default:
    printf_P(PSTR(" ID=%d ***%X_%02X.%02X-%02X"), ID, pu8_cmd[0] & 0x07, pu8_cmd[1], pu8_cmd[2], pu8_cmd[3]);
    break;
  }
  
  //Forward to ESP:  
  fprintf_P(&UART0_str,PSTR("ID=%d, Type=%d, val=%e, Vdd=%f, rtr=%d"),ID, pu8_cmd[0] & 0x07, sensor_val, pu8_cmd[3]/10.0, pu8_cmd[4]);
  fprintf_P(stdout,    PSTR("ID=%d, Type=%d, val=%e, Vdd=%f, rtr=%d"),ID, pu8_cmd[0] & 0x07, sensor_val, pu8_cmd[3]/10.0, pu8_cmd[4]);
}

void debug_mode()
{
  int cmd;
  int finish=0;
  char txt[50];
  uint8_t   pu8_data[33];
  
  UG_FillFrame(0,0,320,12,C_WHITE);
  UG_FillFrame(0,12,320,240,C_BLACK);
	UG_SetForecolor(C_BLACK);
	UG_FontSelect( &RFONT_8X12 );
  sprintf_P(txt,PSTR("Debug mode. Press ESC to exit.")); UG_PutString(2,2,txt);
	UG_SetForecolor(C_WHITE);
	UG_ConsoleSetBackcolor( ILI9341_BLACK );
	UG_ConsoleSetForecolor( ILI9341_WHITE );
	UG_ConsoleSetArea(1,13,318,238);

  while (1)
  {
    if (HasOneMillisecondPassed()) 
    {
      KBD_Read();
      cmd = KBD_GetKey();
      switch (cmd)
      {
        case BTN_ESC: finish=1; break;
      }

      if(nrf24_dataReady())
      {
        printf(" D:");
        memset(pu8_data,0x00,33);
        nrf24_getData(pu8_data);
        DecodeCommand(pu8_data);
      }
    
    }
    if (finish) break;
  }
}

void menu()
{
  #define MENU_DISPLAY_TIME 5000
  int i;
  double Ktemp;
  uint32_t t1,t2;
  char key;
  char txt[50];
  
  UG_FillFrame(0,0,320,240,C_BLACK);
  sprintf_P(txt,PSTR("Menu: ")); UG_PutString(10,60,txt);
  sprintf_P(txt,PSTR("OK  - Debug mode")); UG_PutString(30,80,txt);
  sprintf_P(txt,PSTR("ESC - Clear sensor order EEPROM.")); UG_PutString(30,100,txt);
  sprintf_P(txt,PSTR("A   - Search for sensors.")); UG_PutString(30,120,txt);
  sprintf_P(txt,PSTR("B   - Set sensor order.")); UG_PutString(30,140,txt);
  sprintf_P(txt,PSTR("exiting in")); UG_PutString(90,160,txt);
  
  t1=GetSysTick();
  t2=t1;
  while (1)
  {
    if (HasOneMillisecondPassed()) {KBD_Read();}
    if (Has_X_MillisecondsPassed(MENU_DISPLAY_TIME,&t1)) break;  //exit menu in 5s
    if (Has_X_MillisecondsPassed(1000,&t2))
    {
        sprintf_P(txt,PSTR("%ds"),(MENU_DISPLAY_TIME-(GetSysTick()-t1))/1000); UG_PutString(190,160,txt);      
    }
    key=KBD_GetKey();
    switch (key)
    {
      case BTN_OK:
        debug_mode();
        t1=GetSysTick();
        break;
      case BTN_ESC:
        EraseSensorOrderEEPROM();
        t1=GetSysTick()-MENU_DISPLAY_TIME;
        break;
      case BTN_A:
        printf_P(PSTR("\nSearching sens...\n"));
        gu8_nSensors = search_sensors();
        if (gu8_nSensors > MAXSENSORS) gu8_nSensors=MAXSENSORS;
        printf_P(PSTR("Found %2d        \n"),gu8_nSensors);
        _delay_ms(2000);
        t1=GetSysTick()-MENU_DISPLAY_TIME;
        break;
      case BTN_B:
        ChangSensorOrder();
        t1=GetSysTick()-MENU_DISPLAY_TIME;
        break;
      default:
        break;
    }
  }       
  //Prep display (and reset graph)
  MAX31855_ReadTemperature(2, &Ktemp,NULL);
  DisplayTemp(Ktemp,1);
  for (i=0; i< gu8_nSensors; i++) {gd_oldT[i] = -1;}
}

int main(void)
{
  uint8_t tx_address[5] = {0xD7,0xD7,0xD7,0xD7,0xD7};
  uint8_t rx_address[5] = {0xE7,0xE7,0xE7,0xE7,0xE7};

  uint32_t t1;
  double Ktemp;
  int result;
  
  KBD_Init();
  Systime_Init();
  UART0_Init();
  ADC_Init();
  LCD_Init();
  MAX31855_Init();
  sei();
  
  nrf24_init();                   // init hardware pins
  nrf24_config(2,5);              // Channel #2 , payload length: 5
  nrf24_tx_address(tx_address);   // Set the device addresses
  nrf24_rx_address(rx_address);

  printf_P(PSTR("\nSearching sens..."));
  gu8_nSensors = search_sensors();
  if (gu8_nSensors > MAXSENSORS) gu8_nSensors=MAXSENSORS;
  printf_P(PSTR("Found %2d.\n"),gu8_nSensors);
  result = ReadSensorOrderFromEEPROM();
  switch (result)
  {
    case 1: printf_P(PSTR("\nStored sensor order restored.\n")); break;
    case 0: printf_P(PSTR("\nStored sensor order does not match found sensors completely - consider reordering sensors.\n")); break;
    default: 
      printf_P(PSTR("\nStored sensor order completely out of whack. Run sensor reordering.")); 
      printf_P(PSTR("\nRerunning sensor search..."));
      gu8_nSensors = search_sensors();
      if (gu8_nSensors > MAXSENSORS) gu8_nSensors=MAXSENSORS;
      printf_P(PSTR("Found %2d.\n"),gu8_nSensors);
      break;
  }
  _delay_ms(3000);

  while (1) 
  {
    if (HasOneMillisecondPassed()) {KBD_Read();}
    if (KBD_GetKey()) menu();
    
    if (Has_X_MillisecondsPassed(200,&t1))
    {
      Read_DS18x20_Temperature_OneByOne();   //Read temperatures of hot water storage tank
   
      DisplayHotTankTemperatures();
      
      MAX31855_ReadTemperature(2, &Ktemp,NULL);
      DisplayTemp(Ktemp,0);
    }
  }
}

