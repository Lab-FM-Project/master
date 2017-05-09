#pragma config OSC = INTIO7     // Internal osc, RA6=CLKO, RA7=I/O
#pragma config FCMEN = OFF		// Fail-Safe Clock Monitor disabled 
#pragma config IESO = OFF		// Oscillator Switchover mode disabled 
#pragma config WDT = OFF        // WDT disabled (control through SWDTEN bit)
#pragma config PWRT = ON       // racmod  -> PWRT disabled
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
unsigned short CurrentFreq; // stores current frequency
unsigned char hardmute = 0; // stores mute state
unsigned char Home = 0;

unsigned int LastButtonState[6] = {1, 1, 1, 1, 1, 1}; // stores the previous button states
unsigned int LastChangeTime[6] = {0, 0, 0, 0, 0, 0}; // stores the time since the last change;
unsigned int ButtonState[6] = {1, 1, 1, 1, 1, 1}; // stores the current button states
unsigned int ui; // stores memory address to check FM chip is working
signed char VolControl = 9; // store the volume control

void PICInit(); // PIC initialisation.
unsigned int ButtonRead(int ButtonStateReading, int ButtonIndex);
void FavChannelButton(unsigned int Ptype, int button); //

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
#define XF				1	        // Exit fail
#define PrevChan        PORTGbits.RG0
#define NextChan        PORTGbits.RG1
#define FavChanOne        PORTCbits.RC5
#define FavChanTwo        PORTCbits.RC6
#define FavChanThree      PORTCbits.RC7
#define VolDown           PORTBbits.RB0
#define VolUp         PORTBbits.RB1
#define MUTE         PORTGbits.RG2
#define FMHIGHCHAN		(1080-690)	// Highest FM channel number
#define FMLOWCHAN		(875-690)   // Lowest FM channel number
#define _XTAL_FREQ 8000000

#include <xc.h>
#include <plib/i2c.h>
#include "lcd.h"
#include "eeprom.h"
#include "fm_functions.h"

enum { // Global error numbers
    GERNONE, // No error
    GERWCOL, // I2C write collision
    GERFINT, // Could not initialize FM module
    GERFMID // Could not read chip ID (0x1010)
};

void main(void) {

    PICInit(); // intialise PIC
    Lcd_Write("Setting PIC Init", 1, 1, 1);
    __delay_ms(1000);
    setHardmute(1); // mute the radio while it loads    
    Lcd_Init(); // Intialise LCD Screen     
    Lcd_Write("Setting LCD Init", 1, 1, 1);
    __delay_ms(1000);

    FMvers(&ui); // Check we have comms with FM chip
    if (ui != 0x1010 || FMinit() == XF) Lcd_Write("FM Com Chip Error", 1, 1, 1);
    Lcd_Write("Setting Frequency", 1, 1, 1);
    CurrentFreq = read_EEPROM(0); // get the frequency the radio was at before  
    FMfrequenc(CurrentFreq); // set frequency on FM Chip
    Lcd_Write("Setting Volume", 1, 1, 1);
    setVolume(VolControl); // set volume on FM Chip
    HomeScreen(CurrentFreq); // update the screen 
    setHardmute(0); //turn on sound again
    Lcd_Write("FM radio initialised", 1, 1, 1);
    __delay_ms(1000);

    while (1) //infinite loop
    {
        // this function updates the screen for the timer interupt
        if (Home) {
            HomeScreen(CurrentFreq);
            Home = 0;
        }

        unsigned char PType;
        PType = ButtonRead(NextChan, 0); // read next channel button
        /* if a short press, increment frequency*/
        if (PType == 1) {
            CurrentFreq = CurrentFreq + 1; // increment Frequency
            write_EEPROM(0, CurrentFreq); // update stored frequency on EEPROM
            FMfrequenc(CurrentFreq); // update FM Chip    
            HomeScreen(CurrentFreq); // update Screen
        }
        /* if a long press, seek up */
        if (PType == 2) {
            Lcd_Write("Seeking Up", 1, 1, 1); // display seeking message
            seek('u'); // set FM chip to seek up
            HomeScreen(CurrentFreq); // display home screen
        }
        PType = ButtonRead(PrevChan, 1); // read Previous channel button
        /* if a short press, decrement frequency*/
        if (PType == 1) {
            CurrentFreq = CurrentFreq - 1; // decrement frequency
            write_EEPROM(0, CurrentFreq); // update stored current frequency
            FMfrequenc(CurrentFreq); // update frequency on FM chip           
            HomeScreen(CurrentFreq); // update frequency on screen
        }
        /* if a long press, seek down*/
        if (PType == 2) {
            Lcd_Write("Seeking DOWN", 1, 1, 1); // display seeking message
            seek('d'); // Set FM chip to seek down
            HomeScreen(CurrentFreq); // update frequency on screen
        }
        // read Mute button
        PType = ButtonRead(MUTE, 2);
        /*if short press, toggle mute on/off*/
        if (PType == 1) {
            hardmute = !hardmute; // toggle mute state
            setHardmute(hardmute); //update FM chip           
            DisplayMuteSymbol(hardmute); // update Screen Symbol          
        }
        // read Fav Channel 1 Button (actual button 3)  
        PType = ButtonRead(FavChanOne, 3);
        FavChannelButton(PType, 3);
        // read Fav Channel 2 Button (actual button 4)  
        PType = ButtonRead(FavChanTwo, 4);
        FavChannelButton(PType, 4);
        // read Fav Channel 3 Button (actual button 5)  
        PType = ButtonRead(FavChanThree, 5);
        FavChannelButton(PType, 5);
    }
}
//
// end main ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//

void FavChannelButton(unsigned int Ptype, int button) {
    unsigned int address = (button * 2) - 4; // calculate 2 byte address from button number
    char output[16];
    /*if fav channel button press is short, change frequency to it's stored frequency*/
    if (Ptype == 1) {
        CurrentFreq = read_EEPROM(address); // get saved frequency
        write_EEPROM(0, CurrentFreq); // update stored current frequency          
        FMfrequenc(CurrentFreq); // set frequency on FM chip          
        HomeScreen(CurrentFreq); // update screen
    }
        /*if fav channel button press is long, save current frequency*/
    else if (Ptype == 2) {
        write_EEPROM(address, CurrentFreq); // save current frequency to FM chip  
        // show saved message
        sprintf(output, "Channel %u Saved", button - 2);
        Lcd_Write(output, 1, 1, 1);
        __delay_ms(2000);
        HomeScreen(CurrentFreq); // go back to home screen           
    }
}

void interrupt high_priority CheckButtonPressed() {

    // if volume down button is pressed
    if (INTCONbits.INT0F == 1) {
        VolControl++; // decrement volume
        setVolume(VolControl); // set volume on FM chip
        VolumeScreen(VolControl); // show volume screen with new level
        INTCONbits.INT0F = 0; // clear interrupt flag for this interrupt button
    }
    // if volume up button is pressed
    if (INTCON3bits.INT1F == 1) {
        VolControl--; //increment volume
        setVolume(VolControl); // set volume on FM chip
        VolumeScreen(VolControl); // show Volume screen with new level
        INTCON3bits.INT1F = 0; // clear interrupt flag for this interrupt button
    }
    // if timer0 register overflows
    if (INTCONbits.TMR0IF == 1) {
        T0CONbits.TMR0ON = 0; // stop timer0 module
        delaytime++; // increment time variable
        // if time since last press is 3 seconds, return to home screen
        if (delaytime == 2000) Home = 1;

        // if time variable overflows
        if (delaytime >= 65534) {
            delaytime = 0; // reset time variable
            for (int cunt = 0; cunt < 7; cunt++) {
                LastButtonState[cunt] = 1; // reset previous button states
                ButtonState[cunt] = 1; // reset the current button states
            }
        }

        INTCONbits.TMR0IF = 0; // reset timer0 interrupt flag
        T0CONbits.TMR0ON = 1; // start timer0 again

    }
}

unsigned int ButtonRead(int ButtonStateReading, int ButtonIndex) {
    int HoldDuration; // time since last button state change
    unsigned int PType = 0;
    char output[16];
    // if state has change, store the time    
    if (ButtonStateReading != LastButtonState[ButtonIndex]) LastChangeTime[ButtonIndex] = delaytime;
    HoldDuration = delaytime - LastChangeTime[ButtonIndex]; // calculate button hold duration
    if (HoldDuration > 30) {
        if (ButtonStateReading != ButtonState[ButtonIndex]) {
            ButtonState[ButtonIndex] = ButtonStateReading; //update button state 
            if (ButtonState[ButtonIndex] == 1) PType = 1; // on button release, return short press

        } else if (ButtonState[ButtonIndex] == 0) // if button is presssed
        {
            if (HoldDuration > 2000) // if button is held for more than 2 seconds
            {
                PType = 2; //return long press
                //set state and previous state so short press isn't triggered on exit
                ButtonState[ButtonIndex] = 1;
                ButtonStateReading = 1;
            }
        }
    }
    LastButtonState[ButtonIndex] = ButtonStateReading; // update stored previous button state   
    return PType;
}

void PICInit() {

    OSCCON = 0b01110010; // Select 8 MHz internal oscillator
    ADCON1 = 0b00111111; // Make all ADC/IO pins digital
    TRISA = 0b00000011; // set RA0 and RA1 as inputs for button
    TRISB = 0b00000011; // set RB0 and RB1 for buttons
    TRISC = 0b11111111; // set all port C pins as inputs for buttons
    TRISG = 0b11111111; // set all port G pins as inputs for buttons
    // clear output ports
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
    TMR0L = TIMER_RESET_VALUE; // Load TMR0 register with reset time;
    /* IC2 set up*/
    SSPCON1bits.SSPEN = 1; // enable SSP module
    OpenI2C(MASTER, SLEW_ON);
    SSPADD = 0x3F;

}