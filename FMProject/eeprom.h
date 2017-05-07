
void write_EEPROM(unsigned int address, unsigned short dat);
unsigned short read_EEPROM(unsigned int address);

unsigned short read_EEPROM(unsigned int address) 
{

    unsigned short temp = 0;
    StartI2C(); //Start bit
    WriteI2C(0xA0); //1010 send byte via I2C (device address + Write)
    WriteI2C(address >> 8); //sending higher order address
    WriteI2C(address & 0xFF); //sending lower order address

    RestartI2C();
    WriteI2C(0xA1); // send byte (device address + R)
    temp = ReadI2C();
    AckI2C();
    IdleI2C();
    temp = ((temp << 8) | ReadI2C());// Returns the LSB of the address
    NotAckI2C();
    IdleI2C();
    StopI2C();
    __delay_ms(10);

    return temp;
}

void write_EEPROM(unsigned int address, unsigned short dat) 
{    
    StartI2C(); //Start bits
    WriteI2C(0xA0); //1010 send byte via I2C (device address + Write)
    IdleI2C();
    WriteI2C(address >> 8);
    IdleI2C();   
    WriteI2C(address & 0xFF); //sending lower order address
    IdleI2C();
    WriteI2C(dat >> 8); // send first byte of data
    IdleI2C();
    WriteI2C(dat & 0xFF); // send byte of data
    IdleI2C();
    StopI2C(); //Stop bit.
    __delay_ms(20);
    
    return;
}
