// Blue Nokia 5110
// ---------------
// Signal        (Nokia 5110) LaunchPad pin
// Reset         (RST, pin 1) connected to PA7
// SSI0Fss       (CE,  pin 2) connected to PA3
// Data/Command  (DC,  pin 3) connected to PA6
// SSI0Tx        (Din, pin 4) connected to PA5
// SSI0Clk       (Clk, pin 5) connected to PA2
// 3.3V          (Vcc, pin 6) power
// back light    (BL,  pin 7) not connected, consists of 4 white LEDs which draw ~80mA total
// Ground        (Gnd, pin 8) ground

// Red SparkFun Nokia 5110 (LCD-10168)
// -----------------------------------
// Signal        (Nokia 5110) LaunchPad pin
// 3.3V          (VCC, pin 1) power
// Ground        (GND, pin 2) ground
// SSI0Fss       (SCE, pin 3) connected to PA3
// Reset         (RST, pin 4) connected to PA7
// Data/Command  (D/C, pin 5) connected to PA6
// SSI0Tx        (DN,  pin 6) connected to PA5
// SSI0Clk       (SCLK, pin 7) connected to PA2
// back light    (LED, pin 8) not connected, consists of 4 white LEDs which draw ~80mA total

#include "Nokia5110.h"

#define DC                      (*((volatile unsigned long *)0x40004100))
#define DC_COMMAND              0
#define DC_DATA                 0x40
#define RESET                   (*((volatile unsigned long *)0x40004200))
#define RESET_LOW               0
#define RESET_HIGH              0x80
#define GPIO_PORTA_DIR_R        (*((volatile unsigned long *)0x40004400))
#define GPIO_PORTA_AFSEL_R      (*((volatile unsigned long *)0x40004420))
#define GPIO_PORTA_DEN_R        (*((volatile unsigned long *)0x4000451C))
#define GPIO_PORTA_AMSEL_R      (*((volatile unsigned long *)0x40004528))
#define GPIO_PORTA_PCTL_R       (*((volatile unsigned long *)0x4000452C))
#define SSI0_CR0_R              (*((volatile unsigned long *)0x40008000))
#define SSI0_CR1_R              (*((volatile unsigned long *)0x40008004))
#define SSI0_DR_R               (*((volatile unsigned long *)0x40008008))
#define SSI0_SR_R               (*((volatile unsigned long *)0x4000800C))
#define SSI0_CPSR_R             (*((volatile unsigned long *)0x40008010))
#define SSI0_CC_R               (*((volatile unsigned long *)0x40008FC8))
#define SSI_CR0_SCR_M           0x0000FF00  // SSI Serial Clock Rate
#define SSI_CR0_SPH             0x00000080  // SSI Serial Clock Phase
#define SSI_CR0_SPO             0x00000040  // SSI Serial Clock Polarity
#define SSI_CR0_FRF_M           0x00000030  // SSI Frame Format Select
#define SSI_CR0_FRF_MOTO        0x00000000  // Freescale SPI Frame Format
#define SSI_CR0_DSS_M           0x0000000F  // SSI Data Size Select
#define SSI_CR0_DSS_8           0x00000007  // 8-bit data
#define SSI_CR1_MS              0x00000004  // SSI Master/Slave Select
#define SSI_CR1_SSE             0x00000002  // SSI Synchronous Serial Port
                                            // Enable
#define SSI_SR_BSY              0x00000010  // SSI Busy Bit
#define SSI_SR_TNF              0x00000002  // SSI Transmit FIFO Not Full
#define SSI_CPSR_CPSDVSR_M      0x000000FF  // SSI Clock Prescale Divisor
#define SSI_CC_CS_M             0x0000000F  // SSI Baud Clock Source
#define SSI_CC_CS_SYSPLL        0x00000000  // Either the system clock (if the
                                            // PLL bypass is in effect) or the
                                            // PLL output (default)
#define SYSCTL_RCGC1_R          (*((volatile unsigned long *)0x400FE104))
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))
#define SYSCTL_RCGC1_SSI0       0x00000010  // SSI0 Clock Gating Control
#define SYSCTL_RCGC2_GPIOA      0x00000001  // port A Clock Gating Control

enum typeOfWrite{
  COMMAND,                              // the transmission is an LCD command
  DATA                                  // the transmission is data
};

// Helper function that sends an 8-bit message to the LCD.
// inputs: type     COMMAND or DATA
//         message  8-bit code to transmit
void static lcdwrite(enum typeOfWrite type, char message){
  if(type == COMMAND){
                                        
    while((SSI0_SR_R&SSI_SR_BSY)==SSI_SR_BSY){};
    DC = DC_COMMAND;
    SSI0_DR_R = message;                
                                        
    while((SSI0_SR_R&SSI_SR_BSY)==SSI_SR_BSY){};
  } else{
    while((SSI0_SR_R&SSI_SR_TNF)==0){}; 
    DC = DC_DATA;
    SSI0_DR_R = message;                
  }
}

// Initialize Nokia 5110 48x84 LCD 
void Nokia5110_Init(void){
  volatile unsigned long delay;
  SYSCTL_RCGC1_R |= SYSCTL_RCGC1_SSI0;  
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOA; 
  delay = SYSCTL_RCGC2_R;               
  GPIO_PORTA_DIR_R |= 0xC0;             
  GPIO_PORTA_AFSEL_R |= 0x2C;           
  GPIO_PORTA_AFSEL_R &= ~0xC0;          
  GPIO_PORTA_DEN_R |= 0xEC;             
                                        
  GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R&0xFF0F00FF)+0x00202200;
                                        
  GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R&0x00FFFFFF)+0x00000000;
  GPIO_PORTA_AMSEL_R &= ~0xEC;          
  SSI0_CR1_R &= ~SSI_CR1_SSE;           
  SSI0_CR1_R &= ~SSI_CR1_MS;            
                                        
  SSI0_CC_R = (SSI0_CC_R&~SSI_CC_CS_M)+SSI_CC_CS_SYSPLL;
                                        
  SSI0_CPSR_R = (SSI0_CPSR_R&~SSI_CPSR_CPSDVSR_M)+24; 
  SSI0_CR0_R &= ~(SSI_CR0_SCR_M |       
                  SSI_CR0_SPH |         
                  SSI_CR0_SPO);         
                                        
  SSI0_CR0_R = (SSI0_CR0_R&~SSI_CR0_FRF_M)+SSI_CR0_FRF_MOTO;
                                        
  SSI0_CR0_R = (SSI0_CR0_R&~SSI_CR0_DSS_M)+SSI_CR0_DSS_8;
  SSI0_CR1_R |= SSI_CR1_SSE;            

  RESET = RESET_LOW;                    
  for(delay=0; delay<10; delay=delay+1);
  RESET = RESET_HIGH;                   

  lcdwrite(COMMAND, 0x21);              
                                        
  lcdwrite(COMMAND, CONTRAST);          
  lcdwrite(COMMAND, 0x04);              
  lcdwrite(COMMAND, 0x14);              

  lcdwrite(COMMAND, 0x20);              
  lcdwrite(COMMAND, 0x0C);              
}

//********Nokia5110_OutChar*****************
// Print a character to the Nokia 5110 48x84 LCD. 
// inputs: data  character to print
void Nokia5110_OutChar(unsigned char data){
  int i;
  lcdwrite(DATA, 0x00);
  for(i=0; i<5; i=i+1){
    lcdwrite(DATA, ASCII[data - 0x20][i]);
  }
  lcdwrite(DATA, 0x00);
}

// Print a string of characters to the Nokia 5110 48x84 LCD.
// inputs: ptr  pointer to NULL-terminated ASCII string
void Nokia5110_OutString(char *ptr){
  while(*ptr){
    Nokia5110_OutChar((unsigned char)*ptr);
    ptr = ptr + 1;
  }
}

void Nokia5110_OutUDec(unsigned short n){
  if(n < 10){
    Nokia5110_OutString("    ");
    Nokia5110_OutChar(n+'0'); 
  } else if(n<100){
    Nokia5110_OutString("   ");
    Nokia5110_OutChar(n/10+'0');
    Nokia5110_OutChar(n%10+'0');
  } else if(n<1000){
    Nokia5110_OutString("  ");
    Nokia5110_OutChar(n/100+'0');
    n = n%100;
    Nokia5110_OutChar(n/10+'0'); 
    Nokia5110_OutChar(n%10+'0'); 
  }
  else if(n<10000){
    Nokia5110_OutChar(' ');
    Nokia5110_OutChar(n/1000+'0');
    n = n%1000;
    Nokia5110_OutChar(n/100+'0'); 
    n = n%100;
    Nokia5110_OutChar(n/10+'0'); 
    Nokia5110_OutChar(n%10+'0'); 
  }
  else {
    Nokia5110_OutChar(n/10000+'0'); 
    n = n%10000;
    Nokia5110_OutChar(n/1000+'0'); 
    n = n%1000;
    Nokia5110_OutChar(n/100+'0'); 
    n = n%100;
    Nokia5110_OutChar(n/10+'0'); 
    Nokia5110_OutChar(n%10+'0'); 
  }
}

// Move the cursor to the desired X- and Y-position.
// inputs: newX  new X-position of the cursor (0<=newX<=11)
//         newY  new Y-position of the cursor (0<=newY<=5)
void Nokia5110_SetCursor(unsigned char newX, unsigned char newY){
  if((newX > 11) || (newY > 5)){        // invalid input
    return;                             
  }
  lcdwrite(COMMAND, 0x80|(newX*7));    
  lcdwrite(COMMAND, 0x40|newY);        
}

// Clear the screen
void Nokia5110_Clear(void){
  int i;
  for(i=0; i<(MAX_X*MAX_Y/8); i=i+1){
    lcdwrite(DATA, 0x00);
  }
  Nokia5110_SetCursor(0, 0);
}

// Fill the whole screen by drawing a 48x84 bitmap image.
// inputs: ptr  pointer to 504 byte bitmap
void Nokia5110_DrawFullImage(const char *ptr){
  int i;
  Nokia5110_SetCursor(0, 0);
  for(i=0; i<(MAX_X*MAX_Y/8); i=i+1){
    lcdwrite(DATA, ptr[i]);
  }
}
char Screen[SCREENW*SCREENH/8]; // buffer stores the next image to be printed on the screen

//********Nokia5110_PrintBMP*****************
// Bitmaps defined above were created for the LM3S1968 or
// LM3S8962's 4-bit grayscale OLED display.
// inputs: xpos      horizontal position of bottom left corner of image, columns from the left edge
//                     must be less than 84
//                     0 is on the left; 82 is near the right
//         ypos      vertical position of bottom left corner of image, rows from the top edge
//                     must be less than 48
//                     2 is near the top; 47 is at the bottom
//         ptr       pointer to a 16 color BMP image
//         threshold grayscale colors above this number make corresponding pixel 'on'
//                     0 to 14
//                     0 is fine for ships, explosions, projectiles, and bunkers
void Nokia5110_PrintBMP(unsigned char xpos, unsigned char ypos, const unsigned char *ptr, unsigned char threshold){
  long width = ptr[18], height = ptr[22], i, j;
  unsigned short screenx, screeny;
  unsigned char mask;
  // check for clipping
  if((height <= 0) ||              
     ((width%2) != 0) ||           
     ((xpos + width) > SCREENW) || 
     (ypos < (height - 1)) ||      
     (ypos > SCREENH))           { 
    return;
  }
  if(threshold > 14){
    threshold = 14;             
  }
  // bitmaps are encoded backwards, so start at the bottom left corner of the image
  screeny = ypos/8;
  screenx = xpos + SCREENW*screeny;
  mask = ypos%8;                
  mask = 0x01<<mask;            
  j = ptr[10];                  
  for(i=1; i<=(width*height/2); i=i+1){
    if(((ptr[j]>>4)&0xF) > threshold){
      Screen[screenx] |= mask;
    } else{
      Screen[screenx] &= ~mask;
    }
    screenx = screenx + 1;
    if((ptr[j]&0xF) > threshold){
      Screen[screenx] |= mask;
    } else{
      Screen[screenx] &= ~mask;
    }
    screenx = screenx + 1;
    j = j + 1;
    if((i%(width/2)) == 0){
      if(mask > 0x01){
        mask = mask>>1;
      } else{
        mask = 0x80;
        screeny = screeny - 1;
      }
      screenx = xpos + SCREENW*screeny;
      switch((width/2)%4){      
        case 0: j = j + 0; break;
        case 1: j = j + 3; break;
        case 2: j = j + 2; break;
        case 3: j = j + 1; break;
      }
    }
  }
}

// Clearing the buffer in RAM
void Nokia5110_ClearBuffer(void){int i;
  for(i=0; i<SCREENW*SCREENH/8; i=i+1){
    Screen[i] = 0;          
  }
}

void Nokia5110_DisplayBuffer(void){
  Nokia5110_DrawFullImage(Screen);
}

