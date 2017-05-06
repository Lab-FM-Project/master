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





//Function Prototypes










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
/*unsigned char nextChannel() {

    
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
/*
unsigned char MuteHard() {
    
    
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


