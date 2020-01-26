/*
 * DS18B20management.c
 *
 * Created: 26/01/2020 14:50:10
 *  Author: maticpi
 */ 

#include "DS18B20management.h"

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
