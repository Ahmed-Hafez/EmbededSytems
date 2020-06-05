
#include "ADC.h"
#include "tm4c123gh6pm.h"

// Sets up the ADC 
void ADC0_Init(void){ 
	volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000010;   
  delay = SYSCTL_RCGC2_R;         
  GPIO_PORTE_DIR_R &= ~0x04;      
  GPIO_PORTE_AFSEL_R |= 0x04;     
  GPIO_PORTE_DEN_R &= ~0x04;      
  GPIO_PORTE_AMSEL_R |= 0x04;     
  SYSCTL_RCGC0_R |= 0x00010000;   
  delay = SYSCTL_RCGC2_R;         
  SYSCTL_RCGC0_R &= ~0x00000300;  
  ADC0_SSPRI_R = 0x0123;          
  ADC0_ACTSS_R &= ~0x0008;        
  ADC0_EMUX_R &= ~0xF000;         
  ADC0_SSMUX3_R = (ADC0_SSMUX3_R&0xFFFFFFF0)+1; 
  ADC0_SSCTL3_R = 0x0006;         
  ADC0_ACTSS_R |= 0x0008;         
}

// Analog to digital converter
unsigned long ADC0_In(void){  
  unsigned long result;
  ADC0_PSSI_R = 0x0008;            
  while((ADC0_RIS_R&0x08)==0){};   
  result = ADC0_SSFIFO3_R&0xFFF;   
  ADC0_ISC_R = 0x0008;             
  return result;
}
