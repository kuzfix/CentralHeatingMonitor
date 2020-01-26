/*
 * Graphics.h
 *
 * Created: 26/01/2020 14:53:34
 *  Author: maticpi
 */ 


#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#include "LCD_Ili9341.h"

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

int Hsl_IsValid(float h, float s, float l);
void HSL2RGB(float h, float s, float l,float *R, float *G, float *B);
void PrepDisplay();
void DisplayHotTankTemperatures();
void ClearLastLine(int x);
void DisplayTemp(double T, char reset);
void PrepSensorOrderDisplay();
void DisplaySensorNr(int n);
void DisplaySensorOrderTemperatures(char *order, int imax);
void DisplayNewSensorOrder(char *order);

extern double gd_Temp[MAXSENSORS];
extern double gd_oldT[MAXSENSORS];
extern uint8_t gu8_nSensors;

#endif /* GRAPHICS_H_ */