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
unsigned int evt, delaytime = 0;
int LastButtonState[7];
int LastChangeTime[7];
int ButtonState[7];
unsigned int ui;
int c = 0;
#include <xc.h>
#define _XTAL_FREQ 8000000
#include <plib/i2c.h>
#include "lcd.h"
#include "fm.h"

#include "fm_readwrite.h"
#include "fm_functions.h"
#include "eeprom.h"
unsigned char ButtonRead(int ButtonStateReading, int ButtonIndex);
void FavChannelButton(unsigned char Ptype, int button);

void main(void) {
    //int *eepromfreq;
        //char output[15];

    //unsigned int length;
    Init();
    INTCONbits.GIEH = 0; // Enable High Level interrupts
    INTCONbits.INT0IE = 0; // Enable the INT0 external interrupt
    INTCON3bits.INT1IE = 0; // Enable the INT1 external interrupt

    INTCON2bits.INTEDG0 = 0; // INT0 interrupt on rising edge
    INTCON2bits.INTEDG1 = 0; // INT1 interrupt on rising edge

    INTCONbits.INT0F = 0; //reset interrupt flag
    INTCON3bits.INT1F = 0; //reset interrupt flag
    Lcd_Init();
    
    
    dly(20);
    
    
    Lcd_Clear();
    Lcd_Set_Cursor(1, 1);
    Lcd_Write_String("Checking FM Comms");  
    __delay_ms(1000);
    FMvers(&ui); // Check we have comms with FM chip
    if (ui != 0x1010) errfm();
    if (FMinit() != XS) errfm();
    Lcd_Clear();
    Lcd_Set_Cursor(1, 1);
    Lcd_Write_String("Setting Home Frequency");
    __delay_ms(1000);
    FMfrequenc(964);
    Lcd_Clear();
    Lcd_Set_Cursor(1, 1);
    Lcd_Write_String("Setting Volume");
    __delay_ms(1000);
    setVolume(VolControl);
    for (int count = 0; count < 8; count++)
    {
      LastButtonState[count] = 0;
      LastChangeTime[count] = 0;
      ButtonState[count] = 1;  
    }
     
    //*eepromfreq = 964;
    //write_EEPROM(0x05, 10); 
   // length = read_EEPROM(0x00);
    
    //EERandomRead1(0xA0, 0x00);

    /*OSCCONbits.IRCF0 = 1;
    OSCCONbits.IRCF1 = 1;
    OSCCONbits.IRCF2 = 1;*/
    
  
    Lcd_Clear();
    Lcd_Set_Cursor(1, 1);
    Lcd_Write_String("Getting Current Freq");
    __delay_ms(1000);
    CurrentFreq = frequency();
    Lcd_Clear();
    HomeScreen(CurrentFreq);
    INTCONbits.GIEH = 1; // Enable High Level interrupts
    INTCONbits.INT0IE = 1; // Enable the INT0 external interrupt
    INTCON3bits.INT1IE = 1; // Enable the INT1 external interrupt

    INTCON2bits.INTEDG0 = 1; // INT0 interrupt on rising edge
    INTCON2bits.INTEDG1 = 1; // INT1 interrupt on rising edge

    INTCONbits.INT0F = 0; //reset interrupt flag
    INTCON3bits.INT1F = 0; //reset interrupt flag
    
    T0CON = 0b01000010;				// Prescale by 16
    TMR0L =  TIMER_RESET_VALUE;
    
    INTCONbits.TMR0IF = 0; // Clear timer flag
    INTCONbits.TMR0IE = 1;
    
    T0CONbits.TMR0ON = 1;
   
 

    while (1) //infinite loop
    {
        char output[5];
        unsigned char PType;
        PType = ButtonRead(NextChan, 0);
        if (PType == 1) nextChannel();
        if (PType == 2) {
            Lcd_Clear();
            SeekScreen('u');
            seek('u');
            Lcd_Clear();
            HomeScreen(CurrentFreq);
        }
        PType = ButtonRead(PrevChan, 1);
        if (PType == 1) previousChannel();
        if (PType == 2) {
            Lcd_Clear();
            SeekScreen('d');
            seek('d');
            Lcd_Clear();
            HomeScreen(CurrentFreq);
        }
        PType = ButtonRead(MUTE, 2);
        if (PType == 1) MuteHard();
       PType = ButtonRead(FavChanOne, 3);
       FavChannelButton(PType, 3);
       PType = ButtonRead(FavChanTwo, 4);
       FavChannelButton(PType, 4);
       PType = ButtonRead(FavChanThree, 5);
       FavChannelButton(PType, 5);
        
        
           
            
            
        


}
}
//
// end main ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//
    void FavChannelButton(unsigned char Ptype, int button)
    {
        char output[5];
        if (Ptype == 1)
        {     
            
            
            
         
            CurrentFreq = read_EEPROM(button);
            /*Lcd_Clear();
            Lcd_Set_Cursor(1, 1);
            sprintf(output, "freq: %u", CurrentFreq);
            Lcd_Write_String(output);
            __delay_ms(1000);*/
            FMfrequenc(CurrentFreq);
            HomeScreen(CurrentFreq);
        } else if (Ptype == 2)
        {     
          
           
            //HomeScreen(CurrentFreq);
            /* c++;
            Lcd_Clear();
           Lcd_Set_Cursor(1, 1);
           sprintf(output, "%u", c);
            Lcd_Write_String(output);
            __delay_ms(1000);*/
           write_EEPROM(button, CurrentFreq);
           
           HomeScreen(CurrentFreq);
        }
        
    }

void interrupt high_priority CheckButtonPressed() {

    if (INTCONbits.INT0F == 1) {
        VolControl--;
        setVolume(VolControl);
        VolumeScreen(VolControl);
        INTCONbits.INT0F = 0;
    }

    if (INTCON3bits.INT1F == 1) {
        VolControl++;
        setVolume(VolControl);
        VolumeScreen(VolControl);
        INTCON3bits.INT1F = 0;
    }

    if (INTCONbits.TMR0IF == 1) {
        T0CONbits.TMR0ON = 0;

        delaytime++;

        if (delaytime == 3000) {
            Lcd_Clear();
            HomeScreen(CurrentFreq);
        }

        if (delaytime >= 65534) {
            delaytime = 0;
        }

        INTCONbits.TMR0IF = 0;
        T0CONbits.TMR0ON = 1;

    }
}

unsigned char ButtonRead(int ButtonStateReading, int ButtonIndex) {
    int HoldDuration;
    unsigned char PType = 0;
    if (ButtonStateReading != LastButtonState[ButtonIndex]) LastChangeTime[ButtonIndex] = delaytime;
    HoldDuration = delaytime - LastChangeTime[ButtonIndex];
    if (HoldDuration > 30) {
        if (ButtonStateReading != ButtonState[ButtonIndex]) {
            ButtonState[ButtonIndex] = ButtonStateReading;
            if (ButtonState[ButtonIndex] == 1) PType = 1;

        } else if (ButtonState[ButtonIndex] == 0) {
            if (HoldDuration > 2000) PType = 2;
        }
    }
    LastButtonState[ButtonIndex] = ButtonStateReading;
    
    
   
    return PType;
}
