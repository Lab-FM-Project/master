//********************************************************************
//Constant Definitions
//********************************************************************

/********************************************************************
 *     Function Name:    HDPageWriteI2C                              *
 *     Parameters:       EE memory ControlByte, address and pointer  *
 *     Description:      Writes data string to I2C EE memory         *
 *                       device. This routine can be used for any I2C*
 *                       EE memory device, which uses 2 bytes of     *
 *                       address data as in the 24LC32A/64/128/256.  *
 *                                                                   *  
 ********************************************************************/

unsigned char HDPageWriteI2C(unsigned char HighAdd, unsigned char LowAdd, unsigned char *wrptr) {
    char output[16];
    IdleI2C(); // ensure module is idle
    StartI2C(); // initiate START condition
    while (SSPCON2bits.SEN); // wait until start condition is over 
    if (WriteI2C(EEPROMWRITE)) return XF;
    IdleI2C(); // ensure module is idle
    WriteI2C(HighAdd); // write HighAdd byte to EEPROM 
    IdleI2C(); // ensure module is idle
    WriteI2C(LowAdd); // write LowAdd byte to EEPROM
    IdleI2C();
    WriteI2C(*wrptr);// ensure module is idle
    //putstringI2C ( wrptr );         // pointer to data for page write
    IdleI2C(); // ensure module is idle
    StopI2C(); // send STOP condition
    Lcd_Clear();
    Lcd_Set_Cursor(1, 1);
    sprintf(output, "WRITE COMPLETE");
    Lcd_Write_String(output);
    while (SSPCON2bits.PEN){}; //wait until stop condition if over
    
    return ( 0); // return with no error
}

/********************************************************************
 *     Function Name:    HDSequentialReadI2C                         *
 *     Parameters:       EE memory ControlByte, address, pointer and *
 *                       length bytes.                               *
 *     Description:      Reads data string from I2C EE memory        *
 *                       device. This routine can be used for any I2C*
 *                       EE memory device, which only uses 2 bytes of*
 *                       address data as in the 24xx32 - 24xx512.    *
 *                                                                   *  
 ********************************************************************/

unsigned char HDSequentialReadI2C(unsigned char ControlByte, unsigned char HighAdd, unsigned char LowAdd, unsigned char *rdptr, unsigned char length) {
    IdleI2C(); // ensure module is idle
    StartI2C(); // initiate START condition
    while (SSPCON2bits.SEN); // wait until start condition is over 
    WriteI2C(EEPROMWRITE); // write 1 byte 
    IdleI2C(); // ensure module is idle
    WriteI2C(HighAdd); // WRITE word address to EEPROM
    IdleI2C(); // ensure module is idle
    WriteI2C(LowAdd); // write HighAdd byte to EEPROM
    IdleI2C(); // ensure module is idle
    RestartI2C(); // generate I2C bus restart condition
    while (SSPCON2bits.RSEN); // wait until re-start condition is over 
    WriteI2C(EEPROMREAD); // WRITE 1 byte - R/W bit should be 1 for read
    IdleI2C(); // ensure module is idle
    getsI2C(rdptr, length); // read in multiple bytes
    NotAckI2C(); // send not ACK condition
    while (SSPCON2bits.ACKEN); // wait until ACK sequence is over 
    StopI2C(); // send STOP condition
    while (SSPCON2bits.PEN); // wait until stop condition is over 
    return ( 0); // return with no error
}
