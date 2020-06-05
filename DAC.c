
#include "DAC.h"
#include "tm4c123gh6pm.h"

#define PB3_0 (*((volatile unsigned long *)0x4000503C))

// Initialize 4-bit DAC 
void DAC_Init(void){
	
  GPIO_PORTB_AMSEL_R &= ~0x0F;     
  GPIO_PORTB_PCTL_R &= ~0x0000FFFF;
  GPIO_PORTB_DIR_R |= 0x0F;      
	GPIO_PORTB_DR8R_R |= 0x0F;    
  GPIO_PORTB_AFSEL_R &= ~0x0F;  
  GPIO_PORTB_DEN_R |= 0x0F;     
}


void DAC_Out(unsigned long data){
  PB3_0 = data;
}
