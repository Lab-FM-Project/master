//********************************************************************
//Constant Definitions
//********************************************************************

void write_EEPROM(unsigned int address, unsigned short dat);
unsigned short read_EEPROM(unsigned int address);
/********************************************************************
 *     Function Name:    HDPageWriteI2C                              *
 *     Parameters:       EE memory ControlByte, address and pointer  *
 *     Description:      Writes data string to I2C EE memory         *
 *                       device. This routine can be used for any I2C*
 *                       EE memory device, which uses 2 bytes of     *
 *                       address data as in the 24LC32A/64/128/256.  *
 *                                                                   *  
 ********************************************************************/

/* unsigned char HDPageWriteI2C() {
    StartI2C(); //Start bit
    __delay_ms(10);

    WriteI2C(0x00); //address
    WriteI2C(0x01); //address
    WriteI2C(0x32);    
    AckI2C();

    StopI2C(); //Stop bit.
    __delay_ms(10);
    
    return ( 0); // return with no error
} */

unsigned short read_EEPROM(unsigned int address) {

    /*    unsigned char firstByt;

       StartI2C(); // Begin I2C communication
       IdleI2C(); // Allow the bus to settle

       // Send address of the chip onto the bus
       if (WriteI2C(0xA0)) return XF;
       IdleI2C();
       WriteI2C(address>>8); //sending higher order address
       IdleI2C();
       WriteI2C(address); //sending lower order address
       IdleI2C();
       RestartI2C(); // Initiate a RESTART command
       IdleI2C();
       WriteI2C(0xA1); // Ask for read from FM chip
       IdleI2C();
       firstByt = ReadI2C(); // Returns the MSB byte
       IdleI2C();
       NotAckI2C();
       IdleI2C();
       StopI2C();
       IdleI2C();*/


    

    unsigned short temp = 0;
    char output[5];
    StartI2C(); //Start bit
    //address = (address *2) - 5;
    WriteI2C(0xA0); //1010 send byte via I2C (device address + W)
    WriteI2C(address >> 8); //sending higher order address
    WriteI2C(address & 0xFF); //sending lower order address

    RestartI2C();
    WriteI2C(0xA1); // send byte (device address + R)
    temp = ReadI2C();
    AckI2C();
    IdleI2C();
    temp = ((temp << 8) | ReadI2C());// Returns the LSB of the temp
    NotAckI2C();
    IdleI2C();
    StopI2C();
    __delay_ms(10);
   /* Lcd_Clear();
    sprintf(output, "%u", temp);
    Lcd_Set_Cursor(1, 1);
    Lcd_Write_String(output);*/

    return temp;
}

void write_EEPROM(unsigned int address, unsigned short dat) {
    /*char output[15];
    Lcd_Clear();
    Lcd_Set_Cursor(1, 1);
    sprintf(output, "%u", dat);
    
   Lcd_Write_String(output);//sending higher order address
    Lcd_Set_Cursor(2, 1);
    sprintf(output, "%u", CurrentFreq);
    Lcd_Write_String(output);*/
    
    StartI2C(); //Start bits
    WriteI2C(0xA0);
    IdleI2C();
    WriteI2C(address >> 8);
    IdleI2C();   
    WriteI2C(address & 0xFF); //sending lower order address
    IdleI2C();
    WriteI2C(dat >> 8);
    IdleI2C();
    WriteI2C(dat & 0xFF);// send data (data to be written)
    IdleI2C();
    StopI2C(); //Stop bit.
    __delay_ms(20);
    
    return;
}
//unsigned char HDSequentialReadI2C(unsigned char HighAdd, unsigned char LowAdd, unsigned char *rdptr, unsigned char length) {
//unsigned char HDPageWriteI2C(unsigned char HighAdd, unsigned char LowAdd, unsigned char *wrptr) {

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

/* unsigned char HDSequentialReadI2C() {
    unsigned char dataOUT[16];




    StartI2C();

    WriteI2C(0xA0);
    WriteI2C(0x00);
    WriteI2C(0x00);
    RestartI2C();

    WriteI2C(0xA1);
    dataOUT[0] = ReadI2C();
    Lcd_Clear();
    Lcd_Set_Cursor(1, 1);
    Lcd_Write_String("READ2 COMPLETE");
    AckI2C();

    NotAckI2C();
    StopI2C();


    return ( 0); // return with no error
} */
