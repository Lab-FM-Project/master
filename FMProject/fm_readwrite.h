/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */
unsigned char FMread(unsigned char regAddr, unsigned int *data);
unsigned char FMwrite(unsigned char adr); // Write a new value to a register
void setBitInRegister(unsigned char address, unsigned char bitRegister, unsigned char bitState);
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



