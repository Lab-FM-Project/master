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
#pragma config CP = OFF			// Program memory block not code-protected 

#include <xc.h>
#define _XTAL_FREQ 8000000
#include <plib/i2c.h>
#include "fm.h"
#include "fm_readwrite.h"
#include "fm_functions.h"
//#include "lcd.h"
#include <delays.h>

void DelayHalfSecond() {
    int i;
    for (i = 0; i < 50; i++)
        __delay_ms(1000);
}


void interrupt low_priority CheckButtonPressed() {
    //int vol;
    //check if the interrupt is caused by the pin RB0
    if (INTCONbits.INT0F == 1) {
        PORTCbits.RC6 = 1;
        __delay_ms(1000);
        PORTCbits.RC6 = 0;
        __delay_ms(500);
        INTCONbits.INT0F = 0;
        INTCON3bits.INT1F = 0;
        setVolume(18);

    } else if (INTCON3bits.INT1F == 1) {
        PORTCbits.RC7 = 1;
        __delay_ms(1000);
        PORTCbits.RC7 = 0;
        __delay_ms(500);
        INTCON3bits.INT1F = 0;
        INTCONbits.INT0F = 0;
        setVolume(0);
    }

}

void main(void) {
    int evt;
    unsigned int ui;
    dly(20);
    Init();

    FMvers(&ui); // Check we have comms with FM chip
    if (ui != 0x1010) errfm();
    if (FMinit() != XS) errfm();
    FMfrequenc(964);

    OSCCONbits.IRCF0 = 1;
    OSCCONbits.IRCF1 = 1;
    OSCCONbits.IRCF2 = 1;

    INTCONbits.GIEH = 1; // Enable High Level interrupts
    INTCONbits.INT0IE = 1; // Enable the INT0 external interrupt
    INTCON3bits.INT1IE = 1; // Enable the INT1 external interrupt

    INTCON2bits.INTEDG0 = 1; // INT0 interrupt on rising edge
    INTCON2bits.INTEDG1 = 1; // INT1 interrupt on rising edge

    INTCONbits.INT0F = 0; //reset interrupt flag
    INTCON3bits.INT1F = 0; //reset interrupt flag

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
            case 5: MuteHard();
                // ...
            case 8: errfm();
                break;

            default: break;
        }
    }


}
//
// end main ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//

