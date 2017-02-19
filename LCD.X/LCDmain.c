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
#include <plib/i2c.h>
#include "fm.h"








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



#include "lcd.h"

/*
                         Main application
 */

// This function creates seconds delay. 
// The argument specifies the delay time in seconds
void Delay_Seconds(unsigned char z)
{
    unsigned char x,y;
    for (y = 0; y < z; y++)
    {
        for (x = 0; x < 100; x++)__delay_ms(10);
    }
}
void main(void)
{
    Init();
    
    // initialize the LCD
    LCD_Initialize();


    // If using interrupts in PIC18 High/Low Priority Mode you need to enable the Global High and Low Interrupts
    // If using interrupts in PIC Mid-Range Compatibility Mode you need to enable the Global and Peripheral Interrupts
    // Use the following macros to:

    // Enable high priority global interrupts
    //INTERRUPT_GlobalInterruptHighEnable();

    // Enable low priority global interrupts.
    //INTERRUPT_GlobalInterruptLowEnable();

    // Disable high priority global interrupts
    //INTERRUPT_GlobalInterruptHighDisable();

    // Disable low priority global interrupts.
    //INTERRUPT_GlobalInterruptLowDisable();

    // Enable the Global Interrupts
    //INTERRUPT_GlobalInterruptEnable();

    // Enable the Peripheral Interrupts
    //INTERRUPT_PeripheralInterruptEnable();

    // Disable the Global Interrupts
    //INTERRUPT_GlobalInterruptDisable();

    // Disable the Peripheral Interrupts
    //INTERRUPT_PeripheralInterruptDisable();

     LCDPutStr("  Hello World!");  //Display String "Hello World"
     LCDGoto(8,1);                 //Go to column 8 of second line
     LCDPutChar('1');              //Display character '1'
     Delay_Seconds(1);    // 1 second delay
     LCDGoto(8,1);        //Go to column 8 of second line
     LCDPutChar('2');     //Display character '2'
     Delay_Seconds(1);    // 1 second delay
     LCDGoto(8,1);        //Go to column 8 of second line
     LCDPutChar('3');     //Display character '3'
     Delay_Seconds(1);    // 1 second delay
     DisplayClr();        // Clear the display
     
     LCDPutStr("   LCD Display");  //Display String "LCD Display"                     
             LCDGoto(0,1);         //Go to second line                         
            LCDPutStr("StudentCompanion"); //Display String "StudentCompanion"                     
    while (1)
    {
        // Add your application code
    }
}
/**
 End of File
*/

