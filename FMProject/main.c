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
unsigned int delaytime = 0;
int LastButtonState[6] = {0,0,0,0,0,0};
int LastChangeTime[6]= {0,0,0,0,0,0};
int ButtonState[6]= {1,1,1,1,1,1};
unsigned int ui;

#include <xc.h>
#define _XTAL_FREQ 8000000
#include <plib/i2c.h>
#include "lcd.h"
#include "fm.h"

#include "fm_readwrite.h"
#include "eeprom.h"
#include "fm_functions.h"

unsigned char ButtonRead(int ButtonStateReading, int ButtonIndex);
void FavChannelButton(unsigned char Ptype, int button);

void main(void) {
    //int *eepromfreq;
        //char output[15];

    //unsigned int length;
    Init();
    setHardmute(1);
    INTCONbits.GIEH = 0; // Enable High Level interrupts
    INTCONbits.INT0IE = 0; // Enable the INT0 external interrupt
    INTCON3bits.INT1IE = 0; // Enable the INT1 external interrupt

    INTCON2bits.INTEDG0 = 0; // INT0 interrupt on rising edge
    INTCON2bits.INTEDG1 = 0; // INT1 interrupt on rising edge

    INTCONbits.INT0F = 0; //reset interrupt flag
    INTCON3bits.INT1F = 0; //reset interrupt flag
    Lcd_Init(); // Intialise LCD Screen 
    
    Lcd_Write("Checking FM Comms", 1, 1, 1);
    FMvers(&ui); // Check we have comms with FM chip
    if (ui != 0x1010) errfm();
    if (FMinit() != XS) errfm();
    Lcd_Write("Setting Frequency", 1, 1, 1);
    CurrentFreq = read_EEPROM(0);
    HomeScreen(CurrentFreq);
    FMfrequenc(CurrentFreq);
    Lcd_Write("Setting Volume", 1, 1, 1);
    setVolume(VolControl);
    
    INTCONbits.GIEH = 1; // Enable High Level interrupts
    INTCONbits.INT0IE = 1; // Enable the INT0 external interrupt
    INTCONbits.INT0F = 0; //reset interrupt flag
    INTCONbits.TMR0IF = 0; // Clear timer flag
    INTCONbits.TMR0IE = 1; //Enable TMR0 interupts
    INTCON2bits.INTEDG0 = 1; // INT0 interrupt on rising edge
    INTCON2bits.INTEDG1 = 1; // INT1 interrupt on rising edge
    INTCON3bits.INT1F = 0; //reset interrupt flag
    INTCON3bits.INT1IE = 1; // Enable the INT1 external interrupt
    
    T0CON = 0b11000010; // Enable Timer0, set Prescaler to 8, set Timer0 to 8 8 bit counter;
    TMR0L =  TIMER_RESET_VALUE; // Load TMR0 register with reset time;
    
   
   
    setHardmute(0);

    while (1) //infinite loop
    {
        char output[5];
        unsigned char PType;
        PType = ButtonRead(NextChan, 0);
        if (PType == 1) {
            CurrentFreq = CurrentFreq + 1;
            write_EEPROM(0, CurrentFreq);
            FMfrequenc(CurrentFreq);
            HomeScreen(CurrentFreq);
        }
        
        if (PType == 2) {
            Lcd_Clear();
            SeekScreen('u');
            seek('u');
            Lcd_Clear();
            HomeScreen(CurrentFreq);
        }
        PType = ButtonRead(PrevChan, 1);
        if (PType == 1) {
            CurrentFreq = CurrentFreq - 1;
            write_EEPROM(0, CurrentFreq);
            FMfrequenc(CurrentFreq);
            HomeScreen(CurrentFreq);
        }
    
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
        unsigned int address = (button *2) - 4;
        char output[5];
        if (Ptype == 1)
        {     
                     
            CurrentFreq = read_EEPROM(address);
            write_EEPROM(0, CurrentFreq);            
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
           write_EEPROM(address, CurrentFreq);
           Lcd_Clear();
            Lcd_Set_Cursor(1, 1);
            sprintf(output, "Channel %u Saved", button - 2);
            Lcd_Write_String(output);
            __delay_ms(2000);
           
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
