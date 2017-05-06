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
unsigned int delaytime = 0;

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
// Define register/bit arrays for particular functions
const unsigned int hardmute_bit[2] = {1, 1}; // Register 1 -  xxxx xxxx xxxx xxDx
const unsigned int softmute_bit[2] = {1, 2}; // Register 1 -  xxxx xxxx xxxx xDxx
const unsigned int seek_bit[2] = {3, 14}; // Register 3 -  xDxx xxxx xxxx xxxx
const unsigned int seekup_bit[2] = {3, 15}; // Register 3 -  Dxxx xxxx xxxx xxxx
const unsigned int tune_bit[2] = {2, 9}; // Register 2 -  xxxx xxDx xxxx xxxx
const unsigned int hiloctrl1_bit[2] = {11, 2}; // Register 11 - xxxx xxxx xxxx xDxx
const unsigned int hiloctrl2_bit[2] = {11, 0}; // Register 11 - xxxx xxxx xxxx xxxD
const unsigned int hiloside_bit[2] = {11, 15}; // Register 11 - Dxxx xxxx xxxx xxxx
const unsigned int enable[2] = {0, 0}; // Register 11 - Dxxx xxxx xxxx xxxx
extern unsigned int PageSize;
unsigned short CurrentFreq;
unsigned char hardmute = 0;

int LastButtonState[6] = {0,0,0,0,0,0};
int LastChangeTime[6]= {0,0,0,0,0,0};
int ButtonState[6]= {1,1,1,1,1,1};
unsigned int ui;
signed char VolControl = 9;

void PICInit(); // Processor initialisation.





unsigned char ButtonRead(int ButtonStateReading, int ButtonIndex);
void FavChannelButton(unsigned char Ptype, int button);





#define RSSI_ADDR		0x12	 // Address of the RSSI register
#define ADDR_STATUS		0x13     // Address of the status register
#define CHAN_MASK		0xFE00	 // Bit range of register 2's channel setting
#define SHIFT_READCHAN	7		 // Number of bits to shift to get READCHAN value
#define MASK_ST			0x0008   // Stereo D3 in address 13H
#define MASK_READCHAN	0xFF80   // D7~D15 in address 13H
#define HILO_MASK		0x7FFA	 // D15, D2 and D0 in register 11 - hi/lo injection bits

#define FMI2CADR		0x20		// Address (for writes) of FM module on I2C bus
#define EEPROMREAD      0xA0        // WRITE
#define EEPROMWRITE     0xA1        // READ

#define DEVRD			0x01		// Read not write an I2C device 
#define FMCHIPVERSADR	0x1C		// Address of FM chip version
#define FMCHIPIDADR		0x1B		// Address of FM chip ID  
#define FMCHIPSTSADR	0x13		// Address of FM chip status

#define FMASKMUTE		0x0001		// Register 1, bit 1
#define FMASKTUNE		0x0200		// Register 2, bit 9
#define FMASKSTATUS		0x0020		// Register 0x13, bit 5
#define FMASKSEEK		0x4000		// Register 3, bit 14
#define FMASKRDCHAN		0xFF80		// Register 2, channel number bits

#define XS				0			// Exit success
#define XF				1			// Exit fail

#define NextChan        PORTGbits.RG0
#define PrevChan        PORTGbits.RG1

#define FavChanOne        PORTCbits.RC5
#define FavChanTwo        PORTCbits.RC6
#define FavChanThree      PORTCbits.RC7


#define VolUp           PORTBbits.RB0
#define VolDown         PORTBbits.RB1

#define MUTE            PORTGbits.RG2

#define FMHIGHCHAN		(1080-690)	// Highest FM channel number
#define FMLOWCHAN		(875-690)

#include <xc.h>
#define _XTAL_FREQ 8000000
#include <plib/i2c.h>
#include "lcd.h"
#include "fm_readwrite.h"
#include "eeprom.h"
#include "fm_functions.h"

enum { // Global error numbers
    GERNONE, // No error
    GERWCOL, // I2C write collision
    GERFINT, // Could not initialize FM module
    GERFMID // Could not read chip ID (0x1010)
};


void main(void) {
   
    PICInit();
    setHardmute(1);
    
    Lcd_Init(); // Intialise LCD Screen 
    
    
    FMvers(&ui); // Check we have comms with FM chip
    if (ui != 0x1010) Lcd_Write("FM Com Chip Error", 1, 1, 1);
    FMinit();
    Lcd_Write("Setting Frequency", 1, 1, 1);
    CurrentFreq = read_EEPROM(0);    
    FMfrequenc(CurrentFreq);
    HomeScreen(CurrentFreq);
    Lcd_Write("Setting Volume", 1, 1, 1);
    setVolume(VolControl);
    
    
    
     
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
        if (PType == 1) {
            hardmute = !hardmute;
            setHardmute(hardmute);
            HomeScreen(CurrentFreq);
            DisplayMuteSymbol(hardmute);
        }
            
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
           
            sprintf(output, "Channel %u Saved", button - 2);
            Lcd_Write(output, 1,1,1);
            __delay_ms(2000);
            hardmute = 0;
            
           
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

void PICInit() {

    OSCCON = 0b01110010; // Select 8 MHz internal oscillator
    ADCON1 = 0b00111111; // Make all ADC/IO pins digital
    TRISA = 0b00000011; // RA0 and RA1 pbutton
    TRISB = 0b00000011; // RB0 and RB1 pbutton
    TRISC = 0b11111111; // RC3 and RC4 do the I2C bus
    TRISG = 0b11111111; // RG0, RG1 & RG3 pbutton
    PORTA = 0;
    PORTB = 0;
    PORTC = 0;
    INTCONbits.INT0IE = 0; // Enable the INT0 external interrupt
    INTCON3bits.INT1IE = 0; // Enable the INT1 external interrupt
    INTCON2bits.INTEDG0 = 0; // INT0 interrupt on rising edge
    INTCON2bits.INTEDG1 = 0; // INT1 interrupt on rising edge
    INTCONbits.INT0F = 0; //reset interrupt flag
    INTCON3bits.INT1F = 0; //reset interrupt flag
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
    /* IC2 set up*/
    SSPCON1bits.SSPEN   = 1;    // enable SSP module
    OpenI2C(MASTER, SLEW_ON);
    SSPADD = 0x3F;
    
}