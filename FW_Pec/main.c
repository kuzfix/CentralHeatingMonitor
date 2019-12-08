/*
 * FW_Pec.c
 *
 * Created: 01-Dec-18 11:25:12
 * Author : maticpi
 */ 

#include <avr/io.h>
#include <avr/eeprom.h>
#include "config.h"
#include "kbd.h"
#include "systime.h"
#include "UART0_IRQ.h"
#include "adc.h"
#include "XPT2046.h"
#include "LCD_Ili9341.h"
#include "onewire.h"
#include "ds18x20.h"
#include "crc8.h"
#include "MAX6675.h"

#define MAX_T 400
#define MIN_T 50
#define DIVS  7

#define PGBAR_MIN_X   1
#define PGBAR_MIN_Y   1
#define PGBAR_MAX_X   29
#define PGBAR_MAX_Y   238
#define PGBAR_H       (PGBAR_MAX_Y-PGBAR_MIN_Y+1)
#define GRAPH_MIN_X   (PGBAR_MAX_X+62)
#define GRAPH_MIN_Y   1
#define GRAPH_MAX_X   318
#define GRAPH_MAX_Y   238
#define GRAPH_WX      (GRAPH_MAX_X-GRAPH_MIN_X+1)
#define GRAPH_WY      (GRAPH_MAX_Y-GRAPH_MIN_Y+1)

double gd_Temp[MAXSENSORS];
double gd_oldT[MAXSENSORS];
uint8_t gu8_nSensors;

void Read_DS18x20_Temperature()
{
  int i;
  uint16_t ttemp;

  for (i=0;i<gu8_nSensors;i++)
  {
    gd_oldT[i]=gd_Temp[i];
    if ( DS18X20_read_meas_raw( &gSensorIDs[i][0], &ttemp) != DS18X20_OK ) gd_Temp[i]=-99.9;
    if (ttemp & 0x0800) gd_Temp[i] = (double)((int)(ttemp | 0xf000));
    else gd_Temp[i] = ttemp;
    gd_Temp[i]/=16.0;
  }
  //Trigger next temperature conversion for all the sensors
  DS18X20_start_meas(DS18X20_POWER_PARASITE,NULL);	//broadcast start convert
}

void Read_DS18x20_Temperature_OneByOne()
{
  static int i=-1;
  uint16_t ttemp;

  if (gu8_nSensors == 0) return;  //If no sensors, abort
  
  if (i < 0)
  {
    //Trigger next temperature conversion for all the sensors
    DS18X20_start_meas(DS18X20_POWER_PARASITE,NULL);	//broadcast start convert
    i=0;
  }
  else
  {
    if (i >= gu8_nSensors)
    {
      i = -1;
      return;
    }
    gd_oldT[i]=gd_Temp[i];
    if ( DS18X20_read_meas_raw( &gSensorIDs[i][0], &ttemp) != DS18X20_OK ) gd_Temp[i]=-99.9;
    if (ttemp & 0x0800) gd_Temp[i] = (double)((int)(ttemp | 0xf000));
    else gd_Temp[i] = ttemp;
    gd_Temp[i]/=16.0;
    i++;
  }  
}

void PrepSensorOrderDisplay()
{
  char txt[50];
  
  UG_FillFrame(0,0,320,13,C_WHITE);
  UG_FillFrame(0,13,320,240,C_BLACK);
  UG_FontSelect(&RFONT_8X12);
  UG_SetForecolor(C_BLACK);
  UG_SetBackcolor(C_WHITE);
  sprintf_P(txt,PSTR("Sensor order:"));
  UG_PutString(3,1,txt);
}

void DisplaySensorNr(int n)
{
  char txt[50];
  sprintf_P(txt,PSTR("%d"),n);
  UG_PutString(125,1,txt);
}

void DisplaySensorOrderTemperatures(char *order, int imax)
{
  static int x=0;
  int i,j,y,oy;
  int color;
  
  for (i=0; i<gu8_nSensors; i++)
  {
    y  = 240 - ( (240-12)*(gd_Temp[i]-10) ) / 30;
    oy = 240 - ( (240-12)*(  gd_oldT[i]-10) ) / 30;
    color = C_WHITE;
    if (i == imax) color = C_RED;
    else
    {
      for (j=0; j<gu8_nSensors; j++)
      {
        if (order[j] == i) color = C_GREEN;
      }
    }
    UG_DrawLine(x,oy,x+1,y,color);
  }
  x++;
  if (x > 320)
  {
    UG_FillFrame(0,13,320,240,C_BLACK);
  }
}

void DisplayNewSensorOrder(char *order)
{
  char txt[50];
  int i;

  UG_FillFrame(0,12,320,240,C_BLACK);
  UG_FontSelect(&RFONT_8X12);
  UG_SetForecolor(C_WHITE);
  UG_SetBackcolor(C_BLACK);
  
  for (i=0; i<gu8_nSensors; i++)
  {
    sprintf_P(txt,PSTR("%d. - %d"),i+1,order[i]+1);
    UG_PutString(5,15+12*i,txt);
  }
}

void PrepDisplay()
{
  
  char txt[50];
  int i;
  int y;
  
  UG_FillFrame(0,0,320,240,C_BLACK);
  UG_DrawFrame(PGBAR_MIN_X-1,PGBAR_MIN_Y-1,PGBAR_MAX_X+1,PGBAR_MAX_Y+1,C_CYAN);
  UG_DrawFrame(GRAPH_MIN_X-1,GRAPH_MIN_Y-1,GRAPH_MAX_X+1,GRAPH_MAX_Y+1,C_WHITE);
  UG_FontSelect(&RFONT_8X12);
  UG_SetForecolor(C_WHITE);
  UG_SetBackcolor(C_BLACK);
  sprintf_P(txt,PSTR("%3d"),MAX_T);
  UG_PutString(GRAPH_MIN_X-30,GRAPH_MIN_Y,txt);
  
  sprintf_P(txt,PSTR("%3d"),MAX_T-(MAX_T-MIN_T)/DIVS*2);
  UG_PutString(GRAPH_MIN_X-30,GRAPH_MIN_Y-6+GRAPH_WY/DIVS*2,txt);
  
  sprintf_P(txt,PSTR("%3d"),MIN_T+(MAX_T-MIN_T)/DIVS*2);
  UG_PutString(GRAPH_MIN_X-30,GRAPH_MAX_Y-10-GRAPH_WY/DIVS*2,txt);
  
  sprintf_P(txt,PSTR("%3d"),MIN_T);
  UG_PutString(GRAPH_MIN_X-30,GRAPH_MAX_Y-10,txt);
  
  for (i=1; i<DIVS; i++)
  {
    y=(GRAPH_WY*i)/DIVS;
    UG_DrawLine(GRAPH_MIN_X,y,GRAPH_MAX_X,y,C_GRAY);
  }
  ILI9341_SetScrollArea(GRAPH_MIN_X,GRAPH_WX);
}

void ClearLastLine(int x)
{
  int i,y;
  UG_DrawLine(x,GRAPH_MIN_Y,x,GRAPH_MAX_Y,C_BLACK);
  for (i=1; i<DIVS; i++)
  {
    y=(GRAPH_WY*i)/DIVS;
    UG_DrawPixel(x,y,C_GRAY);
  }
}

void DisplayTemp(double T, char reset)
{
  #define DECIMATION 300L
  char txt[50];
  static int ox,oy;
  static int32_t idx=-1;
  int x,y,color;

  if (reset) idx=-1;
  if (T<100) color = C_WHITE;
  else if (T<120) color = C_ORANGE;
  else if (T<180) color = C_YELLOW;
  else if (T<250) color = C_GREEN;
  else if (T<300) color = C_YELLOW;
  else if (T<360) color = C_ORANGE;
  else color = C_RED;
  
  UG_SetForecolor(C_WHITE);
  UG_SetBackcolor(C_BLACK);
  sprintf_P(txt,PSTR("%5.1f"),T);
  UG_PutString(GRAPH_MIN_X-49,GRAPH_MAX_Y/2-8,txt);
  UG_DrawFrame(GRAPH_MIN_X-51,GRAPH_MAX_Y/2-9,GRAPH_MIN_X-3,GRAPH_MAX_Y/2+3,color);
  
  if (idx == -1)
  {
    PrepDisplay();
    ox=GRAPH_MIN_X;
    oy=GRAPH_MAX_Y - ((double)(T-MIN_T))/((double)(MAX_T-MIN_T))*GRAPH_WY;
  }
  else if (idx < -1 ) {} //what to do in case of overflow?
  else if (idx < GRAPH_WX*DECIMATION)
  {
    if ((idx % DECIMATION) == 0)
    {
      x=ox+1;
      y=GRAPH_MAX_Y - ((double)(T-MIN_T))/((double)(MAX_T-MIN_T))*GRAPH_WY;
      UG_DrawLine(ox,oy,x,y,color);
      ox=x;
      oy=y;
    }
  }
  else
  {
    if ((idx % DECIMATION) == 0)
    {
      y=GRAPH_MAX_Y - ((double)(T-MIN_T))/((double)(MAX_T-MIN_T))*GRAPH_WY;
      if (ox == (GRAPH_MIN_X+GRAPH_WX)) //if wraparround
      {
        x=GRAPH_MIN_X;
        ClearLastLine(x);
        UG_DrawLine(ox,oy,ox,(y+oy)/2,color);
        UG_DrawLine(x,(y+oy)/2,x,y,color);
      }
      else
      {
        x=ox+1;
        ClearLastLine(x);
        UG_DrawLine(ox,oy,x,y,color);
      }
      ox=x;
      oy=y;
      ILI9341_Scroll(GRAPH_WX-(x-GRAPH_MIN_X));
    }
  }
  idx++;
}


int Hsl_IsValid(float h, float s, float l)
{
  if ((h < 0.0) || (h > 360.0)) return 0;
  if ((s < 0.0) || (s > 1.0)) return 0;
  if ((l < 0.0) || (l > 1.0)) return 0;
  return 1;
}

#define HUE_UPPER_LIMIT 360.0

void HSL2RGB(float h, float s, float l,float *R, float *G, float *B)
{
  float c = 0.0, m = 0.0, x = 0.0;
  if (Hsl_IsValid(h, s, l) != 0)
  {
    c = (1.0 - fabs(2 * l - 1.0)) * s;
    m = 1.0 * (l - 0.5 * c);
    x = c * (1.0 - fabs(fmod(h / 60.0, 2) - 1.0));
    if (h < 60.0)
    {
      *R = c + m;
      *G = x + m;
      *B = m;
    }
    else if (h < 120.0)
    {
      *R = x + m;
      *G = c + m;
      *B = m;
    }
    else if (h < 180.0)
    {
      *R = m;
      *G = c + m;
      *B = x + m;
    }
    else if (h < 240.0)
    {
      *R = m;
      *G = x + m;
      *B = c + m;
    }
    else if (h < 300.0)
    {
      *R = x + m;
      *G = m;
      *B = c + m;
    }
    else
    {
      *R = c + m;
      *G = m;
      *B = x + m;
    }
  }
  else
  {
    //		RED=0x0FFF;
    //		GREEN=0x0FFF;
    //		BLUE=0x0FFF;
    //		while(1);
  }
}

void EraseSensorOrderEEPROM()
{
  int i;
  for (i=0; i<(MAXSENSORS*OW_ROMCODE_SIZE+1); i++)
  {
    eeprom_write_byte((uint8_t*)i, 255);
  }
}

void SaveSensorOrderToEEPROM()
{
  int i,j;
  eeprom_write_byte((uint8_t*)0, gu8_nSensors);
  for (i=0; i<gu8_nSensors; i++)
  {
    for (j=0;j<OW_ROMCODE_SIZE;j++)
    eeprom_write_byte((uint8_t*)(i*OW_ROMCODE_SIZE+j+1), gSensorIDs[i][j]);
  }
}

void ChangSensorOrder()
{
  uint8_t SensorIDs[MAXSENSORS][OW_ROMCODE_SIZE];
  char order[MAXSENSORS];
  double dT[MAXSENSORS];
  double maxdT[MAXSENSORS];
  int i,j,k,imax,cnt1s,cnt10ms=0;
  char done=0;

  printf_P(PSTR("Searching sens...\n"));
  gu8_nSensors = search_sensors();
  if (gu8_nSensors > MAXSENSORS) gu8_nSensors=MAXSENSORS;
  printf_P(PSTR("Found %2d."),gu8_nSensors);
  
  Read_DS18x20_Temperature(); //Make sure temperatures are read at least once before oldT are initialized
  PrepSensorOrderDisplay();
  _delay_ms(1000);
  Read_DS18x20_Temperature(); //First Call triggers first conversion, the second call reads the first results
  for (i=0; i<MAXSENSORS; i++) order[i]=-1;
  for (i=0; i<gu8_nSensors; i++)
  {
    for (j=0; j<gu8_nSensors; j++)
    {
      maxdT[j]=0;
    }
    DisplaySensorNr(i+1);
    imax=-1;
    while (1)
    {
      if (HasOneMillisecondPassed())
      {
        cnt10ms++;
        if (cnt10ms >= 10)	//do once every 10 ms
        {
          cnt10ms=0;
          KBD_Read();
          if (KBD_GetKey() != 0) return;
        }
        cnt1s++;
        if (cnt1s >= 1000)	//do once every second
        {
          cnt1s=0;
          Read_DS18x20_Temperature();
          //calculate Temperature rise steepnes
          for (j=0;j<gu8_nSensors;j++)
          {
            dT[j]=gd_Temp[j]-gd_oldT[j];
            if (dT[j]>maxdT[j] && dT[j]>0.5 ) maxdT[j]=dT[j];
          }
          //find sensor with maxdT
          for (j=0; j<gu8_nSensors;j++)
          {
            done=0;
            for (k=0; k<gu8_nSensors; k++)
            {
              if (order[k] == j)
              {
                maxdT[j]=0;
                done=1;
              }
            }
            if (!done)
            {
              if (imax >= 0)
              {
                if (maxdT[j] > maxdT[imax])
                imax=j;
              }
              else
              {
                if (maxdT[j] > 0)
                imax=j;
              }
            }
          }
          //printf("%d",imax);
          //èe je že bil najden senzor z dovolj veliko strmino
          if (imax >= 0)
          {
            //èe je temperatura že zaèela upadati, je identificiran
            if (dT[imax] < 0)
            {
              order[i]=imax;
              for (j=0;j<OW_ROMCODE_SIZE;j++)
              SensorIDs[i][j]=gSensorIDs[imax][j];
              break;
            }
          }
          DisplaySensorOrderTemperatures(order, imax);
        }
      }
    }
  }
  DisplayNewSensorOrder(order);
  //prepiši IDje z novim vrstnim redom
  for (i=0; i<gu8_nSensors; i++)
  {
    for (j=0;j<OW_ROMCODE_SIZE;j++)
      gSensorIDs[i][j]=SensorIDs[i][j];
  }
  SaveSensorOrderToEEPROM();
  _delay_ms(3000);
}

void DisplayHotTankTemperatures()
{
  char txt[50];
  int i,color;
  float r,g,b,h;
  
  for (i=0; i<gu8_nSensors; i++)
  {
    if ((int)gd_oldT[i] != (int)gd_Temp[i])
    {
      h = 300*(1 - (gd_Temp[i]-10)/70.0)-60;
      if (h<0) h+=360;
      HSL2RGB(h,1,0.5,&r,&g,&b);
      color = (((int)(r*31))<<11)+(((int)(g*63))<<5)+(int)(b*31);
      UG_SetBackcolor(color);
      UG_FillFrame(PGBAR_MIN_X, PGBAR_MIN_Y+(i*PGBAR_H)/gu8_nSensors,PGBAR_MAX_X+1,PGBAR_MIN_Y+((i+1)*PGBAR_H)/gu8_nSensors,color);
      sprintf_P(txt,PSTR("%2d"),(int)gd_Temp[i]);
      UG_PutString(PGBAR_MIN_X+5,PGBAR_MIN_Y+((i+0.5)*PGBAR_H)/gu8_nSensors-6,txt);
    }
  }
}

int ReadSensorOrderFromEEPROM()
{
  int result=1;
  int i,j,k, cur_match=0;
  uint8_t nSensors;
  uint8_t SensorIDs[MAXSENSORS][OW_ROMCODE_SIZE];
  int matchOrig[MAXSENSORS];
  int matchStored[MAXSENSORS];
  
  nSensors = eeprom_read_byte((uint8_t*)0);
  if (nSensors != gu8_nSensors) result = 0; //if number of found sensors doesn't match, the result cannot be great.
  if (nSensors > MAXSENSORS) return -1; //test includes empty EEPROM. if number of sensor is more than MAX, something is really wrong! Stop messing with the sensor order.
  
  //Read stored order
  for (i=0; i<nSensors; i++)
  {
    matchStored[i] = -1; 
    for (j=0;j<OW_ROMCODE_SIZE;j++)
    SensorIDs[i][j] = eeprom_read_byte((uint8_t*)(i*OW_ROMCODE_SIZE+j+1));
  }

  for (i=0; i<gu8_nSensors; i++)
  {
    for (k=0; k<nSensors; k++)
    {
      cur_match = 1;
      for (j=0;j<OW_ROMCODE_SIZE;j++)
      {
        if (gSensorIDs[i][j] != SensorIDs[k][j])
        {
          cur_match = 0;
          break;
        }          
      }
      if (cur_match) break;
    }
    if (cur_match) 
    { 
      matchOrig[i] = k;     // Do I really need to know which sensor matches which?
      matchStored[k] = i; 
    }
    else { matchOrig[i] = -1; }
  }
  
  //Reorder sensors which exist
  for (i=0, k=0; i<gu8_nSensors; i++)
  {
    if (matchOrig[i] >= 0)
    {
      for (j=0;j<OW_ROMCODE_SIZE;j++)
      {
        if (matchStored[k] < 0) k++;
        if (k>nSensors) return -1;  //seriously wrong
        gSensorIDs[i][j] = SensorIDs[k][j];
      }
      k++;        
    }
  }   
  
  if (i != k) result = 0; //not the best
  
  return result; //if (result == 1) great
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
  sprintf_P(txt,PSTR("OK  - /")); UG_PutString(30,80,txt);
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
  MAX6675_ReadTemperature(&Ktemp,NULL);
  DisplayTemp(Ktemp,1);
  for (i=0; i< gu8_nSensors; i++) {gd_oldT[i] = -1;}
}

int main(void)
{
  uint32_t t1;
  double Ktemp;
  int result;
  
  KBD_Init();
  Systime_Init();
  UART0_Init();
  ADC_Init();
  LCD_Init();
  MAX6675_Init();
  sei();
    
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
/*  Color test
      Temp[0]=80;
      Temp[2]=10;
      Temp[1]=i;
      i++;
      if (i==80) i=10;
*/      
      DisplayHotTankTemperatures();
      
      MAX6675_ReadTemperature(&Ktemp,NULL);
      DisplayTemp(Ktemp,0);
    }
  }
}

