
void EEwrite(BYTE * addr,BYTE n) {    // non uso puntatore, xché va a finire a 16bit! SPRECONI!

    EEADR = (BYTE)addr;
    EEDATA=n;

    EECON1bits.EEPGD=0;        // Point to Data Memory
    EECON1bits.CFGS=0;        // Access EEPROM
    EECON1bits.WREN=1;

    INTCONbits.GIE = 0;            // disattiva interrupt globali
    EECON2=0x55;         // Write 55h
    EECON2=0xAA;         // Write AAh
    EECON1bits.WR=1;                                    // abilita write.
    INTCONbits.GIE = 1;            // attiva interrupt globali
    do {
        ClrWdt();
        } while(EECON1bits.WR);                            // occupato ? 


    EECON1bits.WREN=0;                                // disabilita write.
  }

BYTE EEread(SHORTPTR addr) {

    EEADR=(BYTE)addr;            // Address to read
    EECON1bits.EEPGD=0;        // Point to Data Memory
    EECON1bits.CFGS=0;        // Access EEPROM
    EECON1bits.RD=1;        // EE Read
    return EEDATA;                // W = EEDATA
    }