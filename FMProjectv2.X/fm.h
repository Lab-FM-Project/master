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

// Define register/bit arrays for particular functions
const unsigned int hardmute_bit[2] = {1, 1}; // Register 1 -  xxxx xxxx xxxx xxDx
const unsigned int softmute_bit[2] = {1, 2}; // Register 1 -  xxxx xxxx xxxx xDxx
const unsigned int seek_bit[2] = {3, 14}; // Register 3 -  xDxx xxxx xxxx xxxx
const unsigned int seekup_bit[2] = {3, 15}; // Register 3 -  Dxxx xxxx xxxx xxxx
const unsigned int tune_bit[2] = {2, 9}; // Register 2 -  xxxx xxDx xxxx xxxx
const unsigned int hiloctrl1_bit[2] = {11, 2}; // Register 11 - xxxx xxxx xxxx xDxx
const unsigned int hiloctrl2_bit[2] = {11, 0}; // Register 11 - xxxx xxxx xxxx xxxD
const unsigned int hiloside_bit[2] = {11, 15}; // Register 11 - Dxxx xxxx xxxx xxxx

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


#define DEVRD			0x01		// Read not write an I2C device 
#define FMCHIPVERSADR	0x1C		// Address of FM chip version
#define FMCHIPIDADR		0x1B		// Address of FM chip ID  
#define FMCHIPSTSADR	0x13		// Address of FM chip status

#define FMASKMUTE		0x0001		// Register 1, bit 1
#define FMASKTUNE		0x0200		// Register 2, bit 9
#define FMASKSTATUS		0x0020		// Register 0x13, bit 5
#define FMASKSEEK		0x4000		// Register 3, bit 14
#define FMASKRDCHAN		0xFF80		// Register 2, channel number bits

#define NextChan        PORTAbits.RA0
#define PrevChan        PORTAbits.RA1

#define VolUp           PORTBbits.RB0
#define VolDown         PORTBbits.RB5

#define MUTE            PORTGbits.RG0


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
void dly(int d);
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

void Init() {
    int i;

	OSCCON = 0b01110010;        	// Select 8 MHz internal oscillator
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
 * setBitInRegister - set a single bit in the registers, such as hard mute.
 *
 * @param address The address of the register in the module that needs 
 *        to be read.
 *
 * @param bitRegister Where to store the reading.
 *
 * @param bit state either ON or OFF. 
 *
 */
void setBitInRegister(unsigned char address, unsigned char bitRegister, unsigned char bitState) {
    // Use bitState to decide which masking to use (to 1 or to 0)
    if (bitState == 1)
        regImg[address] = regImg[address] | (1 << bitRegister);
    else
        regImg[address] = regImg[address] & ~(1 << bitRegister);

    FMwrite(address);
}
//
// end setBitInRegister ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//


// Set seek direction =================================================================

void setSeekDirection(char direction) {
    // Change the direction of the seek function: 'u' - up, 'd' - down
    if (direction == 'd')
        setBitInRegister(seekup_bit[0], seekup_bit[1], 0);
    else if (direction == 'u')
        setBitInRegister(seekup_bit[0], seekup_bit[1], 1);
}

void setHardmute(unsigned char bitState) {
    setBitInRegister(hardmute_bit[0], hardmute_bit[1], bitState);
}

// Seek method ============================================================

unsigned short seek(char direction) {
    /*
     PSEUDO-CODE FROM PROGRAMMING GUIDE
     1 Set hmute Bit 
     2 Clear TUNE Bit 
     3 Set CHAN Bits 
     4 Clear SEEK Bit 
     5 Set SEEKUP/SPACE/BAND/SEEKTH Bits 
     6 Enable SEEK Bit 
     7 Wait STC flag (Seek/Tune Complete, in ?Status? register) 
     8 If SF then tune with AutoHiLo
     9 Clear hmute Bit 
     10 Update  Functions  (optional,  but  remember  to  update  CHAN  with  the  seek in READCHAN before next seek) 
     */

    unsigned short curChannel;
    unsigned short temp = 0;
    unsigned int status;

    // 1 May put in later
    setHardmute(1);
    // 2 Unset tune bit ------------------------------------------------------
    setBitInRegister(tune_bit[0], tune_bit[1], 0);

    // 3 is not required - no override required

    // 4 Unset seek bit ------------------------------------------------------
    setBitInRegister(seek_bit[0], seek_bit[1], 0);

    // 5 Seek attributes - can be set by other functions ---------------------
    setSeekDirection(direction);

    // 6 Set seek bit to enable seek -----------------------------------------
    setBitInRegister(seek_bit[0], seek_bit[1], 1);

    while (temp == 0) {
        if (FMread(FMCHIPSTSADR, &status) != XS) return XF;
        temp = status & 0x0020;
    }

    // 8 Check if tune was successful, tune with Auto Hi Lo if not
    if (FMread(FMCHIPSTSADR, &status) != XS) return XF;
    temp = status & 0x0010;


    if (temp != 0)
        tuneWithAutoHiLo();

    // 9 May put in later
    setHardmute(0);
    // 10 Update CHAN from result (READCHAN) ------------------------------------
    curChannel = frequency() - 690;
    regImg[2] = regImg[2] & CHAN_MASK;
    regImg[2] |= curChannel;
    FMwrite(2);

    return frequency();
}

// Read low-side or high-side LO injection data =======================================

unsigned char readLOInjection(unsigned char loHi) {
    /*  PSEUDO-CODE FOR READING LO/HI INJECTION
        1. Set R11 (clear D15, clear D0/D2) 
        2. Enable TUNE Bit 
        3. Wait STC flag (Seek/Tune Complete, in ?Status? register) 
        4. Get RSSI 
        5. Clear TUNE Bit
        6. Return RSSI
     */

    unsigned char rssi_val = 0;
    unsigned short temp = 0;
    unsigned int status;
    // 1. Set R11 - D15, D2-D0 - clear for low-side, set for high-side
    if (loHi == 1)
        regImg[11] = regImg[11] | ~(0x7FFA);
    else
        regImg[11] = regImg[11] & 0x7FFA; //0x7FFA - HiLow Mask

    FMwrite(11);

    // 2. Set tune bit -----------------------------
    setBitInRegister(tune_bit[0], tune_bit[1], 1);

    // 3. Wait for STC flag to stabilise -----------

    while (temp == 0) {
        if (FMread(FMCHIPSTSADR, &status) != XS) return XF;
        temp = status & 0x0020;
    }

    // 4. Get RSSI value (R0x12, D9-D15)

    if (FMread(FMCHIPSTSADR, &status) != XS) return XF;
    rssi_val = (status & 0xFE00) >> 9;

    // 5. Clear tune bit -----------------------------
    setBitInRegister(tune_bit[0], tune_bit[1], 0);

    return rssi_val;
}

// Tune with auto high-side/low-side injection ========================================

void tuneWithAutoHiLo() {
    // * A WORK IN PROGRESS!! *

    /* PSEUDO-CODE from programming guide
    (1) Set hmute Bit
    (2) Clear TUNE Bit
    (3) Clear SEEK Bit 
    (4) Set BAND/SPACE/CHAN Bits 
    (5) Read Low-side LO Injection
        1. Set R11 ( clear D15, clear D0/D2 ) 
        2. Enable TUNE Bit 
        3. Wait STC flag (Seek/Tune Complete, in ?Status? register) 
        4. Get RSSI (RSSI1) 
        5. Clear TUNE Bit 
    (6) Read High-side LO Injection 
        1. Set R11( set D15, set D0/D2 )
        2. Enable TUNE Bit 
        3. Wait STC flag (Seek/Tune Complete, in ?Status? register) 
        4. Get RSSI (RSSI2) 
        5. Clear TUNE Bit 
    (7) Compare Hi/Lo Side Signal Strength 
        1. If (RSSI2 > RSSI1) Set R11( clear D15, set D0/D2), else Set R11( set D15, clear D0/D2 ) 
    (8)  Enable TUNE Bit
    (9)  Wait STC flag (Seek/Tune Complete, in ?Status? register) 
    (10) Clear hmute Bit 
    (11) Update Functions (optional) 
     */

    unsigned char rssi_lo, rssi_hi;
    unsigned short curChannel, temp = 0;
    unsigned int status;

    // 1. Set hardware mute
    setHardmute(1);
    // 2. Clear tune bit ------------------------------------------------------
    setBitInRegister(tune_bit[0], tune_bit[1], 0);

    // 3. Clear seek bit ------------------------------------------------------
    setBitInRegister(seek_bit[0], seek_bit[1], 0);

    // 4. Set CHAN from READCHAN ----------------------------------------------
    curChannel = frequency() - 690;
    regImg[2] = regImg[2] & CHAN_MASK;
    regImg[2] |= curChannel;
    FMwrite(2);

    // 5. Read low-side LO injection ------------------------------------------
    rssi_lo = readLOInjection(0);

    // 6. Read high-side LO injection -----------------------------------------
    rssi_hi = readLOInjection(1);

    // 7. Compare signal strength, set bits accordingly -----------------------
    if (rssi_hi > rssi_lo) {
        // Set D0 and D2, clear D15 ----------------------------
        setBitInRegister(hiloctrl1_bit[0], hiloctrl1_bit[1], 1);
        setBitInRegister(hiloctrl2_bit[0], hiloctrl2_bit[1], 1);
        setBitInRegister(hiloside_bit[0], hiloside_bit[1], 0);
    } else {
        // Clear D0 and D2, set D15 ----------------------------
        setBitInRegister(hiloctrl1_bit[0], hiloctrl1_bit[1], 0);
        setBitInRegister(hiloctrl2_bit[0], hiloctrl2_bit[1], 0);
        setBitInRegister(hiloside_bit[0], hiloside_bit[1], 1);
    }

    // 8. Enable tune bit -----------------------------------------------------
    setBitInRegister(tune_bit[0], tune_bit[1], 1);

    // 9. Wait for STC flag to stabilise --------------------------------------
    while (temp == 0) {
        if (FMread(FMCHIPSTSADR, &status) == XS);
        temp = status & 0x0020;
    }
    // 10. Clear hardware mute
    setHardmute(0);
}

unsigned short frequency() {
    unsigned int data;
    FMread(ADDR_STATUS, &data);
    return (((data & MASK_READCHAN) >> SHIFT_READCHAN) + 690);
}

/*
 * SetVolume(volume) -  Tune the FM module achieve new volume.  
 *
 *
 * @param volume has a range from 0 to 18 as an integer. 
 *
 * @return XS on success or XF on error.
 *
 */
unsigned char setVolume(int volume) {
    unsigned int dat;
    unsigned int cn; // AR1010 channel number

    // Put volume value in range 0 - 18
    signed char temp_vol = volume;

    if (temp_vol < 0)
        temp_vol = 0;
    else if (temp_vol > 18)
        temp_vol = 18;


    const unsigned char volume_map[19] = {
        0x0F, 0xCF, 0xDF, 0xFF, 0xCB,
        0xDB, 0xFB, 0xFA, 0xF9, 0xF8,
        0xF7, 0xD6, 0xE6, 0xF6, 0xE3,
        0xF3, 0xF2, 0xF1, 0xF0
    };

    // Volume values are held in registers 3 (D7-10) and 14 (D12-15)
    unsigned char volume_setting = volume_map[temp_vol];

    regImg[3] &= 0xF87F; // Zero the bits to change (D7-10)
    regImg[3] |= ((volume_setting & 0x0F) << 7); // Place 4 LSBs of volume at D7-10
    if (FMwrite(3) != XS) return XF;
    regImg[14] &= 0x0FFF; // Zero the bits to change (D12-15)
    regImg[14] |= ((volume_setting & 0xF0) << 8); // Place 4 MSBs of volume at D12-15
    if (FMwrite(14) != XS) return XF;

    do {
        dly(2);
        if (FMready(&dat) != XS) return XF;
    } while (!dat);
    return XS;

}

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

unsigned char FMfrequenc(unsigned int f) {

    unsigned int dat;
    unsigned int cn; // AR1010 channel number

    cn = f - 690;

    // Correct out of lower range
    if (cn < FMLOWCHAN)
        cn = FMLOWCHAN;
    else if (cn > FMHIGHCHAN)
        cn = FMHIGHCHAN;

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

void dly(int d) {

    int i = 0;

    for (; d; --d)
        for (i = 100; i; --i);
}
//
// end dly ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
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
 * @return 	0 if no button has changed state,
 *			1 if button is pushed,
 *			2 if button is released.
 *
 */
int butnEvent(void) {
    int timereturn;
    if (NextChan == 0) //check if the switch is closed
    {
        for (int c = 0; c <= 10; c++)__delay_ms(5); //wait for 100ms 
        if (NextChan == 0) //check if the switch is still closed
        {
            timereturn = 1;
            for (int c = 0; c <= 10; c++)__delay_ms(7);
            if (NextChan == 0) {
                timereturn = 6;
            }
            return timereturn;
        } else {
            timereturn = 0;
            return timereturn;
        }
    }

    if (PrevChan == 0) //check if the switch is closed
    {
        for (int c = 0; c <= 10; c++)__delay_ms(5); //wait for 100ms 
        if (PrevChan == 0) //check if the switch is still closed
        {
            timereturn = 2;
            for (int c = 0; c <= 10; c++)__delay_ms(12);
            if (PrevChan == 0) {
                timereturn = 7;
            }

            return timereturn;

        } else {
            timereturn = 0;
            return timereturn;
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
    return 0;
}
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
    FMfrequenc(964);
    PORTCbits.RC6 = 1;
    delay_10ms(10);
    PORTCbits.RC6 = 0;

    // Etc.
    return XS;
}

unsigned char previousChannel() {

    FMfrequenc(958);
    PORTCbits.RC7 = 1;
    delay_10ms(10);
    PORTCbits.RC7 = 0;
    // Etc.
    return XS;
}

unsigned char VolumeUp() {
    char dir = 'u';
    seek(dir);
    //setVolume(18);
    //setHardmute(1);
    PORTCbits.RC6 = 1;
    delay_10ms(10);
    PORTCbits.RC6 = 0;
    // Etc.
    return XS;
}

unsigned char VolumeDown() {
    char dir = 'd';
    seek(dir);
    //setVolume(10);
    //setHardmute(0);
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

unsigned char SeekUP() {

    PORTCbits.RC7 = 1;
    delay_10ms(10);
    PORTCbits.RC7 = 0;
    // Etc.
    return XS;
}

unsigned char SeekDOWN() {

    PORTCbits.RC7 = 1;
    delay_10ms(10);
    PORTCbits.RC7 = 0;
    // Etc.
    return XS;
}
//
// end nextChan ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//
//

unsigned char saveFavourite() {

    PORTCbits.RC7 = 1;
    delay_10ms(10);
    PORTCbits.RC7 = 0;
    // Etc.
    return XS;
}

//
// end receiveFM.h ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//
//


