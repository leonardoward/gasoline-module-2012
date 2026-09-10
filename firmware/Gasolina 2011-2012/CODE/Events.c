/** ###################################################################
**     Filename  : Events.C
**     Project   : Gasolina
**     Processor : MC9S08DZ60MLC
**     Component : Events
**     Version   : Driver 01.02
**     Compiler  : CodeWarrior HCS08 C Compiler
**     Date/Time : 2/28/2012, 11:20 AM
**     Abstract  :
**         This is user's event module.
**         Put your event handler code here.
**     Settings  :
**     Contents  :
**         CAN1_OnFullRxBuffer - void CAN1_OnFullRxBuffer(void);
**
** ###################################################################*/
/* MODULE Events */


#include "Cpu.h"
#include "Events.h"

/* User includes (#include below this line is not maintained by Processor Expert) */
byte sensorData;
extern byte GasData,cont;

/*
** ===================================================================
**     Event       :  TI1_OnInterrupt (module Events)
**
**     Component   :  TI1 [TimerInt]
**     Description :
**         When a timer interrupt occurs this event is called (only
**         when the component is enabled - <Enable> and the events are
**         enabled - <EnableEvent>). This event is enabled only if a
**         <interrupt service/event> is enabled.
**     Parameters  : None
**     Returns     : Nothing
** ===================================================================
*/
void TI1_OnInterrupt(void)
{
  /* Write your code here ... */
  sensorData=Sensor_GetVal();
  sensorData=sensorData^0b111111;
  cont++;
  if(sensorData==0b100000)
    GasData++;
  else if(sensorData==0b110000)
    GasData+=2;
  else if(sensorData==0b111000 || sensorData==0b010000)
    GasData+=3;
  else if(sensorData==0b011000)
    GasData+=4;
  else if(sensorData==0b011100 || sensorData==0b001000)
    GasData+=5;
  else if(sensorData==0b001100)
    GasData+=6; 
  else if(sensorData==0b001110 || sensorData==0b000100)
    GasData+=7;
  else if(sensorData==0b000110)
    GasData+=8;
  else if(sensorData==0b000111 || sensorData==0b000010)
    GasData+=9;
  else if(sensorData==0b000011)
    GasData+=10;
  else if(sensorData==0b000001)
    GasData+=11; 
  LED_NegVal();
}

/* END Events */

/*
** ###################################################################
**
**     This file was created by Processor Expert 3.07 [04.34]
**     for the Freescale HCS08 series of microcontrollers.
**
** ###################################################################
*/
