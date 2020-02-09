/*
 * Graphics.c
 *
 * Created: 26/01/2020 14:54:01
 *  Author: maticpi
 */ 
#include "Graphics.h"

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

void DisplayTemp(double Tchim, double Tfire, char reset)
{
  #define DECIMATION 300L
  char txt[50];
  static int ox,oy;
  static int32_t idx=-1;
  int x,y,color;

  if (reset) idx=-1;
  if (Tfire<100) color = C_WHITE;
  else if (Tfire<120) color = C_ORANGE;
  else if (Tfire<180) color = C_YELLOW;
  else if (Tfire<250) color = C_GREEN;
  else if (Tfire<300) color = C_YELLOW;
  else if (Tfire<360) color = C_ORANGE;
  else color = C_RED;
  UG_SetForecolor(C_WHITE);
  UG_SetBackcolor(C_BLACK);
  if (Tfire >= 1000.0) sprintf_P(txt,PSTR("%d"),(int)Tfire);
  else sprintf_P(txt,PSTR("%5.1f"),Tfire);
  UG_PutString(GRAPH_MIN_X-49,210,txt);
  UG_DrawFrame(GRAPH_MIN_X-51,210-1,GRAPH_MIN_X-3,210+11,color);
  
  if (Tchim<100) color = C_WHITE;
  else if (Tchim<120) color = C_ORANGE;
  else if (Tchim<180) color = C_YELLOW;
  else if (Tchim<250) color = C_GREEN;
  else if (Tchim<300) color = C_YELLOW;
  else if (Tchim<360) color = C_ORANGE;
  else color = C_RED;
  UG_SetForecolor(C_WHITE);
  UG_SetBackcolor(C_BLACK);
  if (Tchim >= 1000.0) sprintf_P(txt,PSTR("%d"),(int)Tchim);
  else sprintf_P(txt,PSTR("%5.1f"),Tchim);
  UG_PutString(GRAPH_MIN_X-49,GRAPH_MAX_Y/2-8,txt);
  UG_DrawFrame(GRAPH_MIN_X-51,GRAPH_MAX_Y/2-9,GRAPH_MIN_X-3,GRAPH_MAX_Y/2+3,color);

  if (idx == -1)
  {
    PrepDisplay();
    ox=GRAPH_MIN_X;
    oy=GRAPH_MAX_Y - ((double)(Tchim-MIN_T))/((double)(MAX_T-MIN_T))*GRAPH_WY;
  }
  else if (idx < -1 ) {} //what to do in case of overflow?
  else if (idx < GRAPH_WX*DECIMATION)
  {
    if ((idx % DECIMATION) == 0)
    {
      x=ox+1;
      y=GRAPH_MAX_Y - ((double)(Tchim-MIN_T))/((double)(MAX_T-MIN_T))*GRAPH_WY;
      UG_DrawLine(ox,oy,x,y,color);
      ox=x;
      oy=y;
    }
  }
  else
  {
    if ((idx % DECIMATION) == 0)
    {
      y=GRAPH_MAX_Y - ((double)(Tchim-MIN_T))/((double)(MAX_T-MIN_T))*GRAPH_WY;
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

