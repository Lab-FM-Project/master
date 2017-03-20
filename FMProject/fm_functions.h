/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

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
    CurrentFreq = (((data & MASK_READCHAN) >> SHIFT_READCHAN) + 690);
    return CurrentFreq;    
}

unsigned char setHardmute(unsigned char bitState){
    setBitInRegister(hardmute_bit[0], hardmute_bit[1], bitState);
    return XS;
}

// Set seek direction =================================================================
unsigned char setSeekDirection(char direction)
{
	// Change the direction of the seek function: 'u' - up, 'd' - down
	if(direction == 'd')
		setBitInRegister(seekup_bit[0], seekup_bit[1], 0);
	else if(direction == 'u')
		setBitInRegister(seekup_bit[0], seekup_bit[1], 1);
    return XS;
}

unsigned char enableFM(unsigned char bitState){
    setBitInRegister(enable[0], enable[1], bitState);
    return XS;
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


    if (VolControl < 0)  VolControl = 0;
    else if (VolControl > 18) VolControl = 18;


    const unsigned char volume_map[19] = {
        0x0F, 0xCF, 0xDF, 0xFF, 0xCB,
        0xDB, 0xFB, 0xFA, 0xF9, 0xF8,
        0xF7, 0xD6, 0xE6, 0xF6, 0xE3,
        0xF3, 0xF2, 0xF1, 0xF0
    };

    // Volume values are held in registers 3 (D7-10) and 14 (D12-15)
    unsigned char volume_setting = volume_map[VolControl];

    regImg[3] &= 0xF87F; // Zero the bits to change (D7-10)
    regImg[3] |= ((volume_setting & 0x0F) << 7); // Place 4 LSBs of volume at D7-10
    if (FMwrite(3) != XS) return XF;
    regImg[14] &= 0x0FFF; // Zero the bits to change (D12-15)
    regImg[14] |= ((volume_setting & 0xF0) << 8); // Place 4 MSBs of volume at D12-15
    if (FMwrite(14) != XS) return XF;

    do {
        __delay_ms(2);
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
    delay_10ms(2);
    while (FMready(&dat), !dat) __delay_ms(2);
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

unsigned char FMid(unsigned int *id) {

    if (FMread(FMCHIPIDADR, id) != XS) return XF;
    return XS;
} */
//
// end FMid ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//



