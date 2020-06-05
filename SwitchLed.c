
#include "tm4c123gh6pm.h"
#include "SwitchLed.h"

unsigned long PrevRegFire = 0;
unsigned long PrevSpecFire = 0;
unsigned long SuccessLedCount = 0;
unsigned long FailureLedCount = 0;

// Initialize switch inputs and LED outputs
void SwitchLed_Init(void){ 
  volatile unsigned long  delay;
	GPIO_PORTE_AMSEL_R &= ~0x03; 
  GPIO_PORTE_PCTL_R &= ~0x000000FF; 
  GPIO_PORTE_DIR_R &= ~0x03;   
  GPIO_PORTE_AFSEL_R &= ~0x03; 
  GPIO_PORTE_DEN_R |= 0x03;    
	
	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOB; 
  delay = SYSCTL_RCGC2_R;    
  GPIO_PORTB_AMSEL_R &= ~0x30;      
  GPIO_PORTB_PCTL_R &= ~0x00FF0000; 
  GPIO_PORTB_DIR_R |= 0x30;      
  GPIO_PORTB_AFSEL_R &= ~0x30;   
  GPIO_PORTB_DEN_R |= 0x30;      
}

// Input from fire button (PE0)
// Output: 0 or 1 depending on whether button was just pressed (positive logic)
unsigned char Switch_Fire(void){
	 unsigned char FireBool;
   if((GPIO_PORTE_DATA_R&0x01) && (PrevRegFire == 0)){ 
		 FireBool = 1;
	 }
	 else{
			FireBool = 0;
	 }
	 PrevRegFire = GPIO_PORTE_DATA_R&0x01;
	 return FireBool;
}


// Input from special weapons button (PE1)
// Output: 0 or 1 depending on whether button was just pressed (positive logic)
unsigned char Switch_SpecialFire(void){
		unsigned char SpecFireBool;
   if((GPIO_PORTE_DATA_R&0x02) && (PrevSpecFire == 0)){ // just pressed
		 SpecFireBool = 1;
	 }
	 else{
		 SpecFireBool = 0;
	 }
	 PrevSpecFire = GPIO_PORTE_DATA_R&0x02;
	 return SpecFireBool;
}


void Success_LedOn(unsigned long count){
	GPIO_PORTB_DATA_R |= 0x10;
	SuccessLedCount = count;
}

void Failure_LedOn(unsigned long count){
	GPIO_PORTB_DATA_R |= 0x20;
	FailureLedCount = count;
}

unsigned long Success_LedCount(void){
	if(SuccessLedCount)
			SuccessLedCount--;
	
	return SuccessLedCount;
}

unsigned long Failure_LedCount(void){
	if(FailureLedCount)
			FailureLedCount--;
	
	return FailureLedCount;
}

void Success_LedOff(void){
	GPIO_PORTB_DATA_R &= ~0x10;
}

void Failure_LedOff(void){
	GPIO_PORTB_DATA_R &= ~0x20;
	
}
