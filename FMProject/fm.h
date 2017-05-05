//
// FILE     fm.h
// DATE     140128
// WRITTEN  RAC
// PURPOSE  Header for PIC firmware .   $
// LANGUAGE MPLAB C18
// KEYWORDS FM/RECEIVE/EXPERIMENT/PROJECT/TEACH/I2C/PIC
// PROJECT  FM TxRx Lab experiment
// CATEGORY FIRMWARE
// TARGET   PIC18F6490
//
//
//
// See also -
//
//  http://read.pudn.com/downloads142/sourcecode/embed/615356/AR1000FSamplev085.c__.htm
//  

// This is skeletal code which won't run until 'fleshed out'.
// It does give an idea of the setup and initialization required
// of the PIC and the FM tuner module.  The basics of communication
// over the I2C bus are also covered.
// 
//
//



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
signed char VolControl = 9;


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

#define RSSI_ADDR		0x12	 // Address of the RSSI register
#define ADDR_STATUS		0x13     // Address of the status register
#define CHAN_MASK		0xFE00	 // Bit range of register 2's channel setting
#define SHIFT_READCHAN	7		 // Number of bits to shift to get READCHAN value
#define MASK_ST			0x0008   // Stereo D3 in address 13H
#define MASK_READCHAN	0xFF80   // D7~D15 in address 13H
#define HILO_MASK		0x7FFA	 // D15, D2 and D0 in register 11 - hi/lo injection bits

#define XS				0			// Exit success
#define XF				1			// Exit fail

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
#define FALSE			0
#define TRUE			1

enum { // Global error numbers
    GERNONE, // No error
    GERWCOL, // I2C write collision
    GERFINT, // Could not initialize FM module
    GERFMID // Could not read chip ID (0x1010)
};

void Init(); // Processor initialisation.
unsigned char FMread(unsigned char regAddr, unsigned int *data);
unsigned char FMwrite(unsigned char adr); // Write a new value to a register
unsigned char FMinit(); // Initialise the chip
unsigned char FMready(unsigned int *rdy); // Status is ready or busy
unsigned char FMid(unsigned int *id); // Obtain ID number
unsigned char showFreq(void); // Display the current f in MHz
unsigned char FMvers(unsigned int *vsn); // Obtain version number
unsigned char nextChannel();
unsigned char previousChannel();
void delay_10ms(unsigned int n);
void tuneWithAutoHiLo();
unsigned short frequency();
unsigned char WriteFrequencyEEPROM();
unsigned short seek(char direction);
unsigned char setVolume(int volume);
unsigned char FMfrequenc(unsigned int f);
unsigned char setHardmute(unsigned char bitState);
unsigned char setSeekDirection(char direction);
unsigned char enableFM(unsigned char bitState);




extern unsigned int PageSize;

//Function Prototypes




void Init() {

    int i;

    OSCCON = 0b01110010; // Select 8 MHz internal oscillator
    //LCDCON = 0b10001000; // Enab LC controller. Static mode. INTRC clock
    //LCDPS = 0b00110110; // 37 Hz frame frequency
    ADCON1 = 0b00111111; // Make all ADC/IO pins digital
    TRISA = 0b00000011; // RA0 and RA1 pbutton
    TRISB = 0b00000011; // RB0 and RB1 pbutton
    TRISC = 0b11111111; // RC3 and RC4 do the I2C bus
    TRISG = 0b11111111; // RG0, RG1 & RG3 pbutton
    PORTA = 0;
    PORTB = 0;
    PORTC = 0;
    
    SSPCON1bits.SSPEN   = 1;    // enable SSP module


    OpenI2C(MASTER, SLEW_ON);
	//SSPADD  = ((_XTAL_FREQ/4000)/32) - 1;	
    SSPADD = 0x3F;
    //SSPADD = 19;
}
//
// end Init ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//

/*
 * errfm() -  Firmware error.   Call this on a showstopper.
 * @return Never!
 */
unsigned char errfm() {

    return XS;
}
// end errfm ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//

void delay_10ms(unsigned int n) {
    while (n-- != 0) {
        __delay_ms(12);
    }
}


/*
 * Obtain latest change in state for the pushbutton set.
 *
 * @param butn Which button changed.  See fm.h.
 *
 * @return time the button is held for
 *
 */


/*


int butnEvent(void) {
    
    
    
    



    /*   
        
        if (NextChan == 0) //check if the switch is closed
       {
           __delay_ms(100); //wait for 200ms 
           if (NextChan == 0) //check if the switch is still closed
           {
               timereturn = 1;
               __delay_ms(200);//something
           } 
           else timereturn =0;
        
        }    
           if (PrevChan == 0) //check if the switch is closed
       {
           __delay_ms(100); //wait for 200ms 
           if (PrevChan == 0) //check if the switch is still closed
           {
               timereturn = 2;
               __delay_ms(200);//something
           } 
           else timereturn =0;
        
           }
        
    
    
    

    

       if (MUTE == 0) //check if the switch is closed
       {
           __delay_ms(100); //wait for 200ms 
           if (MUTE == 0) //check if the switch is still closed
           {
               timereturn = 3;
               __delay_ms(200); //something
           } 
           else timereturn =0;    
        
        
   } 
    return timereturn;
} */

//
// end butnEvent ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
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

    
    return XS;
}

unsigned char previousChannel() {

    


    return XS;
}

/*unsigned char VolumeUp() 
{
    VolControl++;
    setVolume(VolControl);
    VolumeScreen(VolControl);
    

=======
unsigned char VolumeUp() {
    //VolControl++;
    setVolume(18);
    VolControl++;
    setVolume(VolControl);
    Lcd_Clear();
    VolumeScreen();
    Lcd_Clear();
    HomeScreen();
    //setHardmute(1);


    
    return XS;
}

unsigned char VolumeDown() {
    VolControl--;
    setVolume(VolControl);
    VolumeScreen(VolControl);
    
    return XS;
}*/

unsigned char MuteHard() {
    hardmute = !hardmute;
    setHardmute(hardmute);
    
    return XS;
}



//
// end nextChan ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//
//

void dly(int d) {

    int i = 0;

    for (; d; --d)
        for (i = 100; i; --i);
}
//
// end dly ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//


//
// end receiveFM.h ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//
//


