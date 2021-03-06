
#define _XTAL_FREQ 8000000




#include <xc.h>
#include "lcd.h"


// BEGIN CONFIG
#pragma config OSC = INTIO7     // Internal osc, RA6=CLKO, RA7=I/O
#pragma config FCMEN = OFF		// Fail-Safe Clock Monitor disabled 
#pragma config IESO = OFF		// Oscillator Switchover mode disabled 
#pragma config WDT = ON        // WDT disabled (control through SWDTEN bit)
#pragma config PWRT = OFF       // racmod  -> PWRT disabled
#pragma config MCLRE = ON       // MCLR pin enabled; RG5 input pin disabled
#pragma config XINST = OFF      // Instruction set extension disabled
#pragma config BOREN = ON      // Brown-out controlled by software
#pragma config BORV = 3         // Brown-out voltage set for 2.0V, nominal
#pragma config STVREN = OFF		// Stack full/underflow will not cause Reset
#pragma config CP = OFF
//END CONFIG

int main()
{
  unsigned int a;
  TRISD = 0x00;
  Lcd_Init();
  
  Lcd_Clear();
  display_mute_symbol(1);  
  HomeScreen(104);
  
  PORTDbits.RD1 = 1;
  
  while (1)
  { 
  
  
  
  
  }
    
   }