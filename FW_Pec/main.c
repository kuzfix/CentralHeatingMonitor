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
#include "LED.h"

/* PROTOCOL
ID valH valL Vdd rtr
ID: 7..3 = ID, 2..0 = sensor type
valH - raw value HIGH byte.
valL - raw value LOW byte.
Vdd - power supply voltage
rtr - number of retransmissions of the previous packet (indicates ink quality, lower is better)
*/
#define THIS_STATION_ID  0xFE
#define STYPE_Tds18b20		0x01
#define STYPE_Tsht21			0x02
#define STYPE_Hsht21			0x03
#define STYPE_Tbmp280			0x04
#define STYPE_Pbmp280			0x05
#define STYPE_TankLevel		0x06
//Expanded types (that NRF link doesnt know, but WiFi link does)
#define STYPE_FurnaceTemperatures  0x11
#define STYPE_HeatStorageLevels   0x12

#define HEAT_STORAGE_REPORT_INTERVAL  10000
void ReportHeatStorageLevels()
{
  int i;
  fprintf_P(&UART0_str,PSTR("Type=%d Tlist="),STYPE_HeatStorageLevels);
  for (i=0; i<gu8_nSensors; i++)
  {
    fprintf_P(&UART0_str,PSTR("%.2f;"), gd_Temp[i]);
  }
    fprintf_P(&UART0_str,PSTR("\n"));
}

void ReportFurnaceTemperatures(float Tch, float Tfb)
{
  fprintf_P(&UART0_str,PSTR("Type=%d Tch=%.2f Tfb=%.2f\n"), STYPE_FurnaceTemperatures, Tch, Tfb);
}

void DecodeCommand(uint8_t *pu8_cmd)
{
  uint8_t   ID = pu8_cmd[0]>>3;
  uint8_t   type = pu8_cmd[0] & 0x07;
  uint16_t  raw = (pu8_cmd[1]<<8) | pu8_cmd[2];
  double sensor_val=0;
  double Vdd = pu8_cmd[3]/10.0;
  uint8_t retries = pu8_cmd[4];
//  char txt[50];
  
//  sprintf_P(txt,PSTR("Vdd=%.1fV N_retries=%d"),pu8_cmd[3]/10.0,pu8_cmd[4]);  
  switch (type)
  {
    case STYPE_Tds18b20:
    if (raw == 0xFFFF) sensor_val = -99.9;
    if (raw & 0x0800) sensor_val = (double)((int)(raw | 0xf000));
    else sensor_val = raw;
    sensor_val/=16.0;
//    printf_P(PSTR(" DS18B20: ID=%d T=%.2fC %s\r\n"), ID, sensor_val, txt);
    break;
    
    case STYPE_Tsht21:
    if (raw == 0xFFFF) sensor_val = -99.9;
    sensor_val = -46.85 + 175.72 / 65536.0 * (float)raw;
//    printf_P(PSTR(" SHT21:   ID=%d T=%.2fC %s\r\n"), ID, sensor_val, txt);
    break;
    
    case STYPE_Hsht21:
    if (raw == 0xFFFF) sensor_val = -99.9;
    sensor_val = -6.0 + 125.0 / 65536.0 * (float)raw;
//    printf_P(PSTR(" SHT21:   ID=%d H=%.2f%%RH %s\r\n"), ID, sensor_val, txt);
    break;
    
    case STYPE_Tbmp280:
    if (raw == 0xFFFF) sensor_val = -99.9;
    sensor_val = raw;
//    printf_P(PSTR(" BMP280:  ID=%d T=%.2fC %s\r\n"), ID, sensor_val, txt);
    break;
    
    case STYPE_Pbmp280:
    if (raw == 0xFFFF) sensor_val = -99.9;
    sensor_val = raw;
//    printf_P(PSTR(" BMP280:  ID=%d P=%.2fmBar %s\r\n"), ID, sensor_val, txt);
    break;
    
    default:
//    printf_P(PSTR("Data:=0x%02X %02X %02X %02X %02X"),pu8_cmd[0], pu8_cmd[1], pu8_cmd[2], pu8_cmd[3], pu8_cmd[4]);
    break;
  }
  fprintf_P(&UART0_str,PSTR("Type=%d, ID=%d, val=%e, Vdd=%f, rtr=%d\n"), type, ID, sensor_val, Vdd, retries);
}

int finishDebugMode=0;
void window_1_callback(UG_MESSAGE* msg)
{
  static int espPrgState=0;
  static uint8_t backupPORTD,backupDDRD;
  if (msg->type == MSG_TYPE_OBJECT)
  {
    if (msg->id == OBJ_TYPE_BUTTON)
    {
      switch (msg->sub_id)
      {
        case BTN_ID_0:
        {
          if (msg->event == OBJ_EVENT_RELEASED)
          {
            if (espPrgState == 0)
            {
              espPrgState = !espPrgState;
              printf_P(PSTR("ESP programming mode ENABLED. "));
              backupPORTD=PORTD;
              backupDDRD=DDRD;
              DDRD &= ~(3<<0);
              PORTD |= (3<<0);
            }
            else
            {
              espPrgState = !espPrgState;
              printf_P(PSTR("ESP programming mode DISABLED. "));
              PORTD=backupPORTD;
              DDRD=backupDDRD;
            }
          }
          break;
        }
        case BTN_ID_1:
        {
          if (msg->event == OBJ_EVENT_RELEASED)
          {
            finishDebugMode=1;
          }
          break;
        }
      }
    }
  }
}

#define MAX_OBJECTS 10
void debug_mode()
{
  uint16_t x,y;
  uint8_t pu8_data[33];
  uint32_t timeToReadNRF,timeToReadUserInput;
  UG_WINDOW window; //Window
  UG_BUTTON button1; // Button container
  UG_BUTTON button2; // Button container
  UG_OBJECT obj_buff_wnd[MAX_OBJECTS]; //Object buffer
  
  finishDebugMode=0;
  UG_WindowCreate(&window, obj_buff_wnd, MAX_OBJECTS, window_1_callback);
  UG_WindowSetBackColor(&window,C_BLACK);
  UG_WindowSetTitleText(&window, "Debug mode"); 
  UG_ButtonCreate(&window, &button1, BTN_ID_0,   0, 170, 155, 216);
  UG_ButtonSetBackColor(&window,BTN_ID_0,C_WHITE_SMOKE);
  UG_ButtonSetText(&window, BTN_ID_0, "PROG_ESP");
  UG_ButtonCreate(&window, &button2, BTN_ID_1, 156, 170, 312, 216);
  UG_ButtonSetBackColor(&window,BTN_ID_1,C_WHITE_SMOKE);
  UG_ButtonSetText(&window, BTN_ID_1, "EXIT");
  UG_WindowShow(&window);
  UG_Update();
  
  //UG_DrawFrame(2,17,316,189,C_RED);
  UG_ConsoleSetArea(2,17,316,189);
  
  XPT2046_Init(320, 240);
  XPT2046_setRotation(ROT0);
  
  while (1)
  {
    if (Has_X_MillisecondsPassed(10,&timeToReadUserInput))
    {
      if (XPT2046_isTouching())
      {
        XPT2046_getPosition(&x, &y, MODE_DFR, 0xff);
        UG_DrawPixel(x,y,C_RED);
        UG_TouchUpdate( x, y, TOUCH_STATE_PRESSED );
      }
      else UG_TouchUpdate( -1, -1, TOUCH_STATE_RELEASED );
      //KBD_Read();
      //cmd = KBD_GetKey();
      /*switch (cmd)
      {
        //case BTN3: finish=1; break;
      }*/
      UG_Update();
    }    
    if (Has_X_MillisecondsPassed(100,&timeToReadNRF))
    {
      if(nrf24_dataReady())
      {
        printf(" D:");
        memset(pu8_data,0x00,33);
        nrf24_getData(pu8_data);
        DecodeCommand(pu8_data);
      }
    }
    if (finishDebugMode) break;
  }
  UG_WindowDelete(&window);
}

void menu()
{
  #define MENU_DISPLAY_TIME 5000
  int i;
  double Tchim,Tfire;
  uint32_t t1,t2;
  char key;
  char txt[50];
  
  DisplayMenu();

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
      case BTN3:
        debug_mode();
        DisplayMenu();
        t1=GetSysTick();
        break;
/*      case BTN1:
        EraseSensorOrderEEPROM();
        t1=GetSysTick()-MENU_DISPLAY_TIME;
        break;
*/
      case BTN1:
        printf_P(PSTR("\nSearching sens...\n"));
        gu8_nSensors = search_sensors();
        if (gu8_nSensors > MAXSENSORS) gu8_nSensors=MAXSENSORS;
        printf_P(PSTR("Found %2d        \n"),gu8_nSensors);
        _delay_ms(2000);
        t1=GetSysTick()-MENU_DISPLAY_TIME;
        break;
      case BTN2:
        ChangSensorOrder();
        t1=GetSysTick()-MENU_DISPLAY_TIME;
        break;
      default:
        break;
    }
  }       
  //Prep display (and reset graph)
  MAX31855_ReadTemperature(2, &Tchim,NULL);
  MAX31855_ReadTemperature(1, &Tfire,NULL);
  DisplayTemp(Tchim, Tfire, 1);
  for (i=0; i< gu8_nSensors; i++) {gd_oldT[i] = -1;}
}

int main(void)
{
  uint8_t tx_address[5] = {0xD7,0xD7,0xD7,0xD7,0xD7};
  uint8_t rx_address[5] = {0xE7,0xE7,0xE7,0xE7,0xE7};
  uint8_t pu8_data[33];

  uint32_t measurementTime,StorageLevelReportingTime,TemperaturesReportingTime;
  double ChimneyTemperature=0, FireBoxTemperature=0;
  int result;

  LED_Init();
  KBD_Init();
  Systime_Init();
  UART0_Init(); UCSR0B &=~ (1<<RXEN0);  //Disable RX
  ADC_Init();
  LCD_Init();
#ifdef DISPLAY_FLIPPED
  ILI9341_setRotation(1);
#endif
  MAX31855_Init();
  XPT2046_Init(320,240);
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
    if (HasOneMillisecondPassed()) 
    {
      KBD_Read();
      if(nrf24_dataReady())
      {
        memset(pu8_data,0x00,33);
        nrf24_getData(pu8_data);
        DecodeCommand(pu8_data);
      }
    }
    if (KBD_GetKey()) menu();
    
    if (Has_X_MillisecondsPassed(200,&measurementTime))
    {
      Read_DS18x20_Temperature_OneByOne();   //Read temperatures of hot water storage tank
      DisplayHotTankTemperatures();

      MAX31855_ReadTemperature(1, &FireBoxTemperature,NULL);
      MAX31855_ReadTemperature(2, &ChimneyTemperature,NULL);
//      MAX31855_ReadTemperature(3, &Ktemp,NULL);
      DisplayTemp(ChimneyTemperature, FireBoxTemperature, 0);
      //Clock failure detect - draw a red square if detected: 
      if (XFDCSR & (1<<XFDIF)) UG_FillFrame(GRAPH_MIN_X-10,20,GRAPH_MIN_X-5,25,C_RED);
    }
    if (Has_X_MillisecondsPassed(10000,&StorageLevelReportingTime)) ReportHeatStorageLevels();
    if (Has_X_MillisecondsPassed(1000,&TemperaturesReportingTime)) ReportFurnaceTemperatures(ChimneyTemperature, FireBoxTemperature);
  }
}
