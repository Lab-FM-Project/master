//
// FILE     main.c
// DATE     140128
// WRITTEN  RAC
// PURPOSE  Utilities to communicate with FM receiver IC.   $
// LANGUAGE MPLAB C18
// KEYWORDS USB/I2C/SPARKFUN/FM_MODULE/RECEIVER
// PROJECT  FM TxRx Lab experiment
// CATEGORY UTILITY
// TARGET   Darwin
//
//


// This is skeletal code which won't run until 'fleshed out'.
// It does give an idea of the setup and initialization required
// of the PIC and the FM tuner module.  The basics of communication
// over the I2C bus are also covered.
// 
// This version contains revisions needed to compile under MPLABX.
// 
// 

#pragma config OSC = INTIO7     // Internal osc, RA6=CLKO, RA7=I/O
#pragma config FCMEN = OFF		// Fail-Safe Clock Monitor disabled 
#pragma config IESO = OFF		// Oscillator Switchover mode disabled 
#pragma config WDT = OFF        // WDT disabled (control through SWDTEN bit)
#pragma config PWRT = OFF       // racmod  -> PWRT disabled
#pragma config MCLRE = ON       // MCLR pin enabled; RG5 input pin disabled
#pragma config XINST = OFF      // Instruction set extension disabled
#pragma config BOREN = OFF      // Brown-out controlled by software
#pragma config BORV = 3         // Brown-out voltage set for 2.0V, nominal
#pragma config STVREN = OFF		// Stack full/underflow will not cause Reset
#pragma config CP = OFF // Program memory block not code-protected 

unsigned short CurrentFreq;
unsigned char hardmute = 0;
int evt, LastButtonState = 1, ButtonState, delaytime = 0 , LastChangeTime = 0;
unsigned int ui;
#include <xc.h>
#define _XTAL_FREQ 8000000
#include <plib/i2c.h>
#include "lcd.h"
#include "fm.h"

#include "fm_readwrite.h"
#include "fm_functions.h"












void main(void) {
    
    dly(20);
    Init();
      
     
    
    
    
        
    
     
    
    FMvers(&ui); // Check we have comms with FM chip
    if (ui != 0x1010) errfm();
    if (FMinit() != XS) errfm();
    FMfrequenc(964);
    
    

    /*OSCCONbits.IRCF0 = 1;
    OSCCONbits.IRCF1 = 1;
    OSCCONbits.IRCF2 = 1;*/

    INTCONbits.GIEH = 1; // Enable High Level interrupts
    INTCONbits.INT0IE = 1; // Enable the INT0 external interrupt
    INTCON3bits.INT1IE = 1; // Enable the INT1 external interrupt

    INTCON2bits.INTEDG0 = 1; // INT0 interrupt on rising edge
    INTCON2bits.INTEDG1 = 1; // INT1 interrupt on rising edge

    INTCONbits.INT0F = 0; //reset interrupt flag
    INTCON3bits.INT1F = 0; //reset interrupt flag*/
    
    CurrentFreq = frequency();
    Lcd_Init();
    HomeScreen(CurrentFreq);
    setVolume(VolControl);
    
    while (1) //infinite loop
    {
        //actually we have to put the processor in sleep which i will cover
        //  in later tutorials
       evt = butnEvent();
        switch (evt) {
            case 1: nextChannel();
                break;
            case 2: previousChannel();
                break;
            case 3: 
                MuteHard();
                break;
            case 4:
                Lcd_Clear();
                SeekScreen();
                seek('u');
                Lcd_Clear();
                HomeScreen(CurrentFreq);
                break;
            case 5:
                Lcd_Clear();
                SeekScreen();
                seek('d');
                Lcd_Clear();
                HomeScreen(CurrentFreq);
                break;
                
            case 8:         
                errfm();
                break;
            default: break;
        } 
        
    }

    
}
//
// end main ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//

void interrupt high_priority CheckButtonPressed() {
    //int vol;
    //check if the interrupt is caused by the pin RB0
    if (INTCONbits.INT0F == 1) {      
        
        VolControl--; 
        setVolume(VolControl);
        VolumeScreen(VolControl);
        
        INTCONbits.INT0F = 0;
        
        
        
        
        

    } if (INTCON3bits.INT1F == 1) {
        
        VolControl++; 
        setVolume(VolControl);
        VolumeScreen(VolControl);
        
        INTCON3bits.INT1F = 0;

        
        
        
    }
    
    if (INTCONbits.TMR0IF == 1) 
    {
        T0CONbits.TMR0ON = 0;
        //char output [16];//INTCONbits.TMR0IE = 0; 
        
        delaytime++;
        
        /*if (delaytime == 3000)
        {
            
            Lcd_Clear();
            HomeScreen(CurrentFreq);
        }*/
        
        if (delaytime >= 10000000)
        {
            delaytime = 0;
            
        }
        INTCONbits.TMR0IF = 0;
        T0CONbits.TMR0ON = 1;
               
        
                    
                    
        
                                           
        
             
  
  
    
    

}
}

