 #include <xc.h>

static bit gie;

void FlashEraseRow( const void *address )
{
    gie = GIE, GIE = 0;
    WREN = 1;
    FREE = 1;            // erasing
    CFGS = 0;            // program memory space
    PMADR = (unsigned short) address;
    PMCON2 = 0x55;
    PMCON2 = 0xAA;
    WR = 1;
    NOP();
    NOP();
    // processor stalls until operation is complete (2 mS typical)
    WREN = 0;
    GIE = gie;
}

void FlashWriteRow( const void *address, void *data )
{
    unsigned char i;

    gie = GIE, GIE = 0;
    WREN = 1;
    FREE = 0;            // writing
    CFGS = 0;            // program memory space

    for( i = 0; i < 16; i++ )
    {
        PMADR = (unsigned short) address + i;
        PMDAT = ((unsigned char *) data)[i];
        LWLO = i != 15;        // clear on final iteration, set otherwise
        PMCON2 = 0x55;
        PMCON2 = 0xAA;
        WR = 1;
        NOP();
        NOP();
        // processor stalls on final iteration until operation is complete (2 mS typical)
    }

    WREN = 0;
    GIE = gie;
}

void FlashReadRow( const void *address, void *data )
{
    unsigned char i;

    CFGS = 0;            // program memory space

    for( i = 0; i < 16; i++ )
    {
        PMADR = (unsigned short) address + i;
        RD = 1;
        NOP();
        NOP();
        ((unsigned char *) data)[i] = PMDAT;
    }
}
 