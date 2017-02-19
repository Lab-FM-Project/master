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
// This version contains eevisions needed to compile under MPLABX.
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

#include<xc.h>
#define _XTAL_FREQ 8000000
#include <plib/i2c.h>
#include "fm.h"

void main(void) {
    int evt;
    unsigned int ui;
    dly(20);
    Init();

    PORTCbits.RC6 = 1;

    FMvers(&ui); // Check we have comms with FM chip
    if (ui != 0x1010) errfm();
    if (FMinit() != XS) errfm();
    FMfrequenc(964);
    for (;;) {

        evt = butnEvent();
        switch (evt) {
            case 1: nextChannel();
                break;
            case 2: previousChannel();
                break;
            case 3: VolumeUp();
                break;
            case 4: VolumeDown();
                break;
            case 5: MuteHard(FALSE);
                break;
            case 6: SeekUP();
                break;
            case 7: SeekDOWN();
                break;
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

