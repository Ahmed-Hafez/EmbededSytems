
#include "tm4c123gh6pm.h"
#include "Nokia5110.h"
#include "SwitchLed.h"
#include "Sound.h"
#include "Random.h"
#include "ADC.h"
#include "GameEngine.h"
#include "TExaS.h"

unsigned char GameOverFlag;
unsigned char Semaphore = 0;

void DisableInterrupts(void); 
void EnableInterrupts(void);  
void Timer2_Init(void(*task)(void), unsigned long period);
void Delay100ms(unsigned long count); 
void PF1Init(void);
void SysTick_Init(unsigned long period); 
void (*PeriodicTask)(void);   

int main(void){
	DisableInterrupts();
  TExaS_Init(SSI0_Real_Nokia5110_Scope);
	Random_Init(1);
  Nokia5110_Init();
	PF1Init();
	SysTick_Init(2666666*4); // making the game run at a playable speed
  Nokia5110_ClearBuffer();
	Nokia5110_DisplayBuffer();
	ADC0_Init();
	Game_Init();
	SwitchLed_Init();
	Sound_Init();
	Timer2_Init(&Sound_Play,7256);
	GameOverFlag = 0;
	EnableInterrupts();
	
  while(1){
		while(Semaphore==0){};
    Semaphore = 0;
		if(GameOverFlag){
			GameOver();
		}
		else{
			Drawing_GameFrame(); // update the LCD
		}	
		if((GameOverFlag == 0) && (Checking_GameOver())){ 
			Delay100ms(2);
			GameOverFlag = Checking_GameOver();
			SysTick_Init(2666666*4); // making the game run at a playable speed
		}
	}
}

void PF1Init(void){
	volatile unsigned long  delay;
	SYSCTL_RCGC2_R |= 0x00000020;   
  delay = SYSCTL_RCGC2_R;           
  GPIO_PORTF_AMSEL_R &= ~0x02;      
  GPIO_PORTF_PCTL_R &= ~0x000000F0; 
  GPIO_PORTF_DIR_R |= 0x02;         
  GPIO_PORTF_AFSEL_R &= ~0x02;      
  GPIO_PORTF_DEN_R |= 0x02;         
}

void SysTick_Init(unsigned long period){
	NVIC_ST_CTRL_R = 0;         
  NVIC_ST_RELOAD_R = period-1;
  NVIC_ST_CURRENT_R = 0;      
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x20000000; 
  NVIC_ST_CTRL_R = 0x0007;  
}
void SysTick_Handler(void){  
	GPIO_PORTF_DATA_R ^= 0x02; 
	//Game methods
	if(GameOverFlag){
		if(Switch_Fire() || Switch_SpecialFire()){
			GameOverFlag = 0;
			Game_Init();
		}
	}
	else{
		Checking_Collisions();
		Move_Objects();  
		if(Switch_Fire()){
			RegMissile_Fire();
			Sound_Shoot();
		}
		if(Switch_SpecialFire()){
			SpecMissile_Fire();
			Sound_Shoot();
		}
		SysTick_Init(Set_Difficulty());
	}
  Semaphore = 1;
}


// Activate Timer2 interrupts to run user task 
// Inputs:  task is a pointer to a user function
//          period in units (1/clockfreq)
void Timer2_Init(void(*task)(void), unsigned long period){
  unsigned long volatile delay;
  SYSCTL_RCGCTIMER_R |= 0x04;   
  delay = SYSCTL_RCGCTIMER_R;
  PeriodicTask = task;          
  TIMER2_CTL_R = 0x00000000;    
  TIMER2_CFG_R = 0x00000000;    
  TIMER2_TAMR_R = 0x00000002;   
  TIMER2_TAILR_R = period-1;    
  TIMER2_TAPR_R = 0;            
  TIMER2_ICR_R = 0x00000001;    
  TIMER2_IMR_R = 0x00000001;    
  NVIC_PRI5_R = (NVIC_PRI5_R&0x00FFFFFF)|0x80000000; 
  NVIC_EN0_R = 1<<23;           
  TIMER2_CTL_R = 0x00000001;    
}

void Timer2A_Handler(void){
	unsigned long checkSuccess = Success_LedCount();
	unsigned long checkFailure = Failure_LedCount();
   TIMER2_ICR_R = 0x00000001;   
	if(!checkSuccess) 
		Success_LedOff();
	
	if(!checkFailure)
		Failure_LedOff();
	
  (*PeriodicTask)();      
}

void Delay100ms(unsigned long cnt){unsigned long volatile timer;
  while(cnt>0){
    timer = 727240;  
    while(timer){
	  	timer--;
    }
    cnt--;
  }
}
