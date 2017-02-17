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
#include "types.h"


// FM register bank defaults -
const unsigned int regDflt[18] = {
    0xFFFF, // R0 -- the first writable register .  (disable xo_en)   
    0x5B15, // R1.   
    0xD0B9, // R2.   
    0xA010, // R3   seekTHD = 16   
    0x0780, // R4   
    0x28AB, // R5   
    0x6400, // R6   
    0x1EE7, // R7   
    0x7141, // R8   
    0x007D, // R9   
    0x82C6, // R10  disable wrap   
    0x4F55, // R11. <--- (disable xo_output)   
    0x970C, // R12.   
    0xB845, // R13   
    0xFC2D, // R14   
    0x8097, // R15   
    0x04A1, // R16   
    0xDF6A // R17
};

unsigned int regImg[18]; // FM register bank images

/*
 * Obtain latest change in state for the pushbutton set.
 *
 * @param butn Which button changed.  See fm.h.
 *
 * @return 	0 if no button has changed state,
 *			1 if button is pushed,
 *			2 if button is released.
 *
 */
int butnEvent(void) {

    if (NextChan == 0) //check if the switch is closed
    {
        for (int c = 0; c <= 10; c++)__delay_ms(5); //wait for 100ms 
        if (NextChan == 0) //check if the switch is still closed
        {
            PORTCbits.RC6 = 1;
            delay_10ms(10);
            PORTCbits.RC6 = 0;
            return 1;

        } else {
            return 0;
        }
    }

    if (PrevChan == 0) //check if the switch is closed
    {
        for (int c = 0; c <= 10; c++)__delay_ms(5); //wait for 100ms 
        if (PrevChan == 0) //check if the switch is still closed
        {
            return 2; //something
        } else {
            return 0; //something
        }
    }

    if (VolUp == 0) //check if the switch is closed
    {
        for (int c = 0; c <= 10; c++)__delay_ms(5); //wait for 100ms 
        if (VolUp == 0) //check if the switch is still closed
        {
            return 3; //something
        } else {
            return 0; //something
        }
    }

    if (VolDown == 0) //check if the switch is closed
    {
        for (int c = 0; c <= 10; c++)__delay_ms(5); //wait for 100ms 
        if (VolDown == 0) //check if the switch is still closed
        {
            return 4; //something
        } else {
            return 0; //something
        }
    }

    if (MUTE == 0) //check if the switch is closed
    {
        for (int c = 0; c <= 10; c++)__delay_ms(5); //wait for 100ms 
        if (MUTE == 0) //check if the switch is still closed
        {
            return 5; //something
        } else {
            return 0; //something
        }
    }

}
//
// end butnEvent ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//

void dly(int d) {

    int i = 0;

    for (; d; --d)
        for (i = 100; i; --i);
}
//
// end dly ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//

/*
 * Set all LCD segments to 0 (off, clear).
 *
 */
void clrscn() {

    int i = 0;
    unsigned char *CLEARptr; // Pointer used to clear all LCDDATA


    for (i = 0,
            CLEARptr = (unsigned char *) &LCDDATA0; // Point to first segment
            i < 28;
            i++) // Turn off all segments
        *CLEARptr++ = 0x00;
}
//
// end clrscn ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//

void Init() {

    int i;

    OSCCON = 0b01110010; // Select 8 MHz internal oscillator
    LCDSE0 = 0b11111111; // Enable  LCD segments 07-00
    LCDSE1 = 0b11111111; // Enable  LCD segments 15-08
    LCDSE2 = 0b11111111; // Enable  LCD segments 23-16
    LCDSE3 = 0b00000000; // Disable LCD segments 31-24
    LCDCON = 0b10001000; // Enab LC controller. Static mode. INTRC clock
    LCDPS = 0b00110110; // 37 Hz frame frequency
    ADCON1 = 0b00111111; // Make all ADC/IO pins digital
    TRISA = 0b00000011; // RA0 and RA1 pbutton
    TRISB = 0b00100001; // RB0 and RB5 pbutton
    TRISC = 0b00011000; // RC3 and RC4 do the I2C bus
    TRISG = 0b11111111; // RG0, RG1 & RG3 pbutton
    PORTA = 0;
    PORTB = 0;
    PORTC = 0;
    INTCONbits.TMR0IF = 0; // Clear timer flag
    //T0CON = 0b00000011;				// Prescale by 16
    T0CON = 0b00001000; // No prescale
    TMR0H = 0; // Clear timer count
    TMR0L = 0;
    T0CONbits.TMR0ON = 1; // Start timer
    OpenI2C(MASTER, SLEW_OFF);
    SSPADD = 0x3F;

    return;
}
//
// end Init ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//

/*
 * Write an individual LCD segment.  
 *
 * @param segOrd The segment ordinal.  Between 0 and 22.
 *
 * @param state Whether to turn the segment dark (true) or clear (false).
 *
 */
void segWrt(unsigned char segOrd, unsigned char state) {

    unsigned char bitSelect;
    unsigned char *LCReg;

    if (segOrd > 23) return;
    LCReg = (unsigned char *) &LCDDATA0 + (segOrd >> 3);
    bitSelect = 1 << (segOrd & 0x07);
    if (state) *LCReg |= bitSelect; // Segment on
    else *LCReg &= ~bitSelect; // Segment off
}
//
// end segWrt ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//

/*
 * FMwrite() -  Write a two byte word to the FM module.  The new 
 * register contents are obtained from the image bank.
 *
 * @param adr The address of the register in the FM module that needs 
 * to be written.
 *
 *
 * @return XS on success or XF on error.
 *
 */
unsigned char FMwrite(unsigned char adr) {

    unsigned int regstr;
    unsigned char firstByt;
    unsigned char secndByt;
    unsigned char rpy;

    firstByt = regImg[adr] >> 8;
    secndByt = regImg[adr];

    StartI2C(); // Begin I2C communication
    IdleI2C();

    // Send slave address of the chip onto the bus
    if (WriteI2C(FMI2CADR)) return XF;
    IdleI2C();
    WriteI2C(adr); // Adress the internal register
    IdleI2C();
    WriteI2C(firstByt); // Ask for write to FM chip
    IdleI2C();
    WriteI2C(secndByt);
    IdleI2C();
    StopI2C();
    IdleI2C();
    return XS;
}
//
// end FMwrite ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//
//

/*
 * FMread - Read a two byte register from the FM module.
 *
 * @param regAddr The address of the register in the module that needs 
 *        to be read.
 *
 * @param data Where to store the reading.
 *
 * @return XS on success or XF on error.
 *
 */
unsigned char FMread(unsigned char regAddr, unsigned int *data) {

    unsigned char firstByt;
    unsigned char secndByt;

    StartI2C(); // Begin I2C communication
    IdleI2C(); // Allow the bus to settle

    // Send address of the chip onto the bus
    if (WriteI2C(FMI2CADR)) return XF;
    IdleI2C();
    WriteI2C(regAddr); // Adress the internal register
    IdleI2C();
    RestartI2C(); // Initiate a RESTART command
    IdleI2C();
    WriteI2C(FMI2CADR + DEVRD); // Ask for read from FM chip
    IdleI2C();
    firstByt = ReadI2C(); // Returns the MSB byte
    IdleI2C();
    AckI2C(); // Send back Acknowledge
    IdleI2C();
    secndByt = ReadI2C(); // Returns the LSB of the temperature
    IdleI2C();
    NotAckI2C();
    IdleI2C();
    StopI2C();
    IdleI2C();
    *data = firstByt;
    *data <<= 8;
    *data = *data | secndByt;

    return XS;
}
//
// end FMread ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//

/*
 * FMready - See if the FM module is ready.
 *
 * @param rdy Where to store the busy/ready status.  Will become
 * non-zero if the chip is ready, zero if busy.
 * 
 *
 *
 * @return XS on success or XF on error.
 *
 */
unsigned char FMready(unsigned int *rdy) {

    unsigned int sts;

    if (FMread(FMCHIPSTSADR, &sts) != XS) return XF;
    sts &= FMASKSTATUS;
    *rdy = sts ? TRUE : FALSE;
    return XS;
}
//
// end FMready ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//

/*
 * FMinit() -  Initialise the FM module.  
 *
 *
 * @return XS on success or XF on error.
 *
 */
unsigned char FMinit() {

    unsigned char ad;
    unsigned int dat;

    // Copy default FM register values to the image set -
    for (ad = 0; ad < 18; ad++) regImg[ad] = regDflt[ad];

    dat = regImg[0];
    regImg[0] &= ~1;
    if (FMwrite(0) != XS) return XF;
    for (ad = 1; ad < 18; ad++) {
        if (FMwrite(ad) != XS)return XF;
    }

    regImg[0] = dat | 1;
    if (FMwrite(0) != XS) return XF;
    dly(20);
    while (FMready(&dat), !dat) dly(2);
    showFreq();
    return XS;
}
//
// end FMinit ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//
//

/*
 * FMfrequenc(f) -  Tune the FM module to new frequency.  
 *
 *
 * @param f The new frequency as a multiple of 100 kHz.
 *
 * @return XS on success or XF on error.
 *
 */
unsigned char setVolume(int volume) {
    unsigned int dat;
    unsigned int cn; // AR1010 channel number

    // Put volume value in range 0 - 18
	unsigned char temp_vol = volume;
	
	if(temp_vol < 0)
		temp_vol = 0;
	else if(temp_vol > 18)
		temp_vol = 18;
    

    const unsigned char volume_map[19] = {
        0x0F, 0xCF, 0xDF, 0xFF, 0xCB,
        0xDB, 0xFB, 0xFA, 0xF9, 0xF8,
        0xF7, 0xD6, 0xE6, 0xF6, 0xE3,
        0xF3, 0xF2, 0xF1, 0xF0
    };
    
    // Volume values are held in registers 3 (D7-10) and 14 (D12-15)
	unsigned char volume_setting = volume_map[temp_vol];

	regImg[3] &= 0xF87F;		// Zero the bits to change (D7-10)
	regImg[3] |= ((volume_setting & 0x0F) << 7);    // Place 4 LSBs of volume at D7-10
	if (FMwrite(3) != XS) return XF;
    regImg[14] &= 0x0FFF;		// Zero the bits to change (D12-15)
	regImg[14] |= ((volume_setting & 0xF0) << 8);      // Place 4 MSBs of volume at D12-15
	if (FMwrite(14) != XS) return XF;

    do {
        dly(2);
        if (FMready(&dat) != XS) return XF;
    } while (!dat);
    return XS;

}

unsigned char FMfrequenc(unsigned int f) {

    unsigned int dat;
    unsigned int cn; // AR1010 channel number

    cn = f - 690;

    // NB AR1010 retunes on 0 to 1 transition of TUNE bit -	
    regImg[2] &= ~FMASKTUNE;
    if (FMwrite(2) != XS) return XF;
    regImg[2] &= 0xfe00;
    regImg[2] |= (cn | FMASKTUNE);
    if (FMwrite(2) != XS) return XF;
    do {
        dly(2);
        if (FMready(&dat) != XS) return XF;
    } while (!dat);
    return XS;
}
//
// end FMfrequenc ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//
//

/*
 * FMvers - Obtain the FM chip version.
 *
 * @param vsn Where to store the version number.  Will become
 * 0x65B1 for vintage 2009 devices.
 *
 * @return XS on success or XF on error. *
 */
unsigned char FMvers(unsigned int *vsn) {
    if (FMread(FMCHIPVERSADR, vsn) != XS) return XF;
    return XS;
}
//
// end FMvers ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//

/*
 * FMid - Obtain the FM chip ID.
 * * @param id Where to store the ID number.  Will become
 * 0x1010 for AR1010 devices.
 *
 * @return XS on success or XF on error. *
 */
unsigned char FMid(unsigned int *id) {

    if (FMread(FMCHIPIDADR, id) != XS) return XF;
    return XS;
}
//
// end FMid ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//

/*
 * nextChan() -  Tune to the next channel.
 *
 * @param up Set to non-zero for next channel up,
 *  zero for preset down.
 *
 * @return XS on success or XF on error.
 *
 */
unsigned char nextChannel() {
    FMfrequenc(964);
    PORTCbits.RC6 = 1;
    delay_10ms(10);
    PORTCbits.RC6 = 0;

    // Etc.
    return XS;
}

unsigned char previousChannel() {
    FMfrequenc(1046);
    PORTCbits.RC7 = 1;
    delay_10ms(10);
    PORTCbits.RC7 = 0;
    // Etc.
    return XS;
}

unsigned char VolumeUp() {
    setVolume(18);
    PORTCbits.RC6 = 1;
    delay_10ms(10);
    PORTCbits.RC6 = 0;
    // Etc.
    return XS;
}

unsigned char VolumeDown() {
    setVolume(10);
    PORTCbits.RC7 = 1;
    delay_10ms(10);
    PORTCbits.RC7 = 0;
    // Etc.
    return XS;
}

unsigned char MuteHard(unsigned char down) {

    // Etc.
    return XS;
}
//
// end nextChan ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//
//

/*
 * errfm() -  Firmware error.   Call this on a showstopper.
 *
 *
 * @return Never!
 *
 */
void errfm() {

    ; // Do something helpful
    for (;;);
}
//
// end errfm ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//
//

/*
 * Display the frequency that the receiver chip is set to.
 *
 * @return XS if successful, else XF on failure.
 *
 */
unsigned char showFreq() {

    ; // Etc
    return XS;
}
//
// end showFreq ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//

void main(void) {
    int evt;
    unsigned int ui;
    dly(20);
    Init();


    FMvers(&ui); // Check we have comms with FM chip
    if (ui != 0x1010) errfm();
    if (FMinit() != XS) errfm();

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
                // ...
            case 8: errfm();
                break;

            default: break;
        }
    }
}

void delay_10ms(unsigned int n) {
    while (n-- != 0) {
        __delay_ms(12);
    }
}
//
// end main ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//

