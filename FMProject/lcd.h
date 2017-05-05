
//LCD Functions Developed by electroSome

#include <stdio.h>
#define RS PORTDbits.RD2
#define EN PORTDbits.RD3
#define D4 PORTDbits.RD4
#define D5 PORTDbits.RD5
#define D6 PORTDbits.RD6
#define D7 PORTDbits.RD7
#define TIMER_RESET_VALUE 6 /*Calculated by the formula
                            //TMR1 = 256 - (timerPeriod*Fosc)/(4*prescaler) + x

                            //In this case, timerPeriod = 0.001s
                            //              Fosc = 8,000,000
                            //              prescaler = 8
                            //              x = 0 because PS is > 2

                            //So,   TMR1 = 256 - (0.001*8000000)/(4*8)
                            //      TMR1 = 256 - 8000/16
                            //      TMR1 = 6 */







int delayTime = 0; //A global variable to store the time that has
//elapsed. This must be global!
void Store_mute_symbol();
void Lcd_Port(char a);
void Lcd_Cmd(char a);
void Lcd_Clear();
void Lcd_Set_Cursor(char a, char b);
void Lcd_Init();
void Lcd_Write_Char(char a);
void Lcd_Write_String(char *a);
void Lcd_Shift_Right();
void Lcd_Shift_Left();
void DisplayMuteSymbol(unsigned char muted);
void HomeScreen(unsigned short freq);
void VolumeScreen (int level);
void SeekScreen(char direction);


void Lcd_Write_Char(char a) {
     char temp,y;
   temp = a&0x0F;
   y = a&0xF0;
   RS = 1;             // => RS = 1
   Lcd_Port(y>>4);             //Data transfer
   EN = 1;
   __delay_us(60);
   EN = 0;
   Lcd_Port(temp);
   EN = 1;
   __delay_us(60);
    EN = 0;
}

void Lcd_Port(char a) {
    if (a & 1)
        D4 = 1;
    else
        D4 = 0;

    if (a & 2)
        D5 = 1;
    else
        D5 = 0;

    if (a & 4)
        D6 = 1;
    else
        D6 = 0;

    if (a & 8)
        D7 = 1;
    else
        D7 = 0;
}

void Lcd_Cmd(char a) {
    RS = 0;             // => RS = 0
	Lcd_Port(a);
	EN  = 1;             // => E = 1
        __delay_ms(4);
        EN  = 0;             // => E = 0
}

void Lcd_Clear() {
    Lcd_Cmd(0);
    Lcd_Cmd(1);
}

void Lcd_Set_Cursor(char a, char b) {
    char temp, z, y;
    if (a == 1) {
        temp = 0x80 + b - 1;
        z = temp >> 4;
        y = temp & 0x0F;
        Lcd_Cmd(z);
        Lcd_Cmd(y);
    } else if (a == 2) {
        temp = 0xC0 + b - 1;
        z = temp >> 4;
        y = temp & 0x0F;
        Lcd_Cmd(z);
        Lcd_Cmd(y);
    }
}

void Store_mute_symbol()
 {

    Lcd_Cmd(0x04);
    Lcd_Cmd(0x00); // set the address to CGRAM
    Lcd_Write_Char(0x02);
    Lcd_Write_Char(0x06);
    Lcd_Write_Char(0x1e);
    Lcd_Write_Char(0x1e);
    Lcd_Write_Char(0x1e);
    Lcd_Write_Char(0x1e);
    Lcd_Write_Char(0x06);
    Lcd_Write_Char(0x02);



}

void Lcd_Init() {
  TRISD = 0x00;
  Lcd_Port(0x00);
   __delay_ms(40);
  Lcd_Cmd(0x03);
	__delay_ms(10);
  Lcd_Cmd(0x03);
	__delay_ms(22);
  Lcd_Cmd(0x03);
  /////////////////////////////////////////////////////
  Lcd_Cmd(0x02);
  Lcd_Cmd(0x02);
  Lcd_Cmd(0x08);
  Lcd_Cmd(0x00);
  Lcd_Cmd(0x0C);
  Lcd_Cmd(0x00);
  Lcd_Cmd(0x06);
      
  Store_mute_symbol();
  Lcd_Clear();
  
    
    
 
//T1CONbits.TMR1ON = 0;
   //configure T0CON register
  //T1CONbits.RD16 = 1;
    /* 16-Bit Read/Write Mode Enable bit 
    1 = Enables register read/write of Timer3 in one 16-bit operation
    0 = Enables register read/write of Timer3 in two 8-bit operations
     */

   //T1CONbits.T1RUN = 0;
    /*Timer1 System Clock Status bit
    1 = Device clock is derived from Timer1 oscillator
    0 = Device clock is derived from another source */

   //T1CONbits.T1CKPS1 = 1, T1CONbits.T1CKPS0 = 1;
    /*Timer1 Input Clock Prescale Select bits
    11 = 1:8 Prescaler value
    10 = 1:4 Prescaler value
    01 = 1:2 Prescaler value
    00 = 1:1 Prescaler value*/
    //T1CONbits.T1OSCEN = 0;
    /*Timer1 Oscillator Enable bit
    1 = Timer1 oscillator is enabled
    0 = Timer1 oscillator is shut off
    The oscillator inverter and feedback resistor are turned off to eliminate power drain.*/
   //T1CONbits.T1SYNC = 1;
    /*Timer1 External Clock Input Synchronization Select bit
    When TMR1CS = 1:
    1 = Do not synchronize external clock input
    0 = Synchronize external clock input
    When TMR1CS = 0:
    This bit is ignored. Timer1 uses the internal clock when TMR1CS = 0*/
   //T1CONbits.TMR1CS = 0;
    /*Timer1 Clock Source Select bit
    1 = External clock from pin RC0/T1OSO/T13CKI (on the rising edge)
    0 = Internal clock (FOSC/4)*/

    //configure other registers
    
    //PIR1bits.TMR1IF = 0; // clear Interupt Flag
    //PIE1bits.TMR1IE = 1; // enable interupt
    
    
}



void Lcd_Write_String(char *a) {
    int i;
	for(i=0;a[i]!='\0';i++)
	   Lcd_Write_Char(a[i]);
}

/*void Lcd_Shift_Right() {
    Lcd_Cmd(0x01);
    Lcd_Cmd(0x0C);
}

void Lcd_Shift_Left() {
    Lcd_Cmd(0x01);
    Lcd_Cmd(0x08);
}*/

void DisplayMuteSymbol(unsigned char muted) {
    Lcd_Set_Cursor(1, 15);
    Lcd_Write_Char(0x00);
    Lcd_Set_Cursor(1, 16);
    if (muted == 1) Lcd_Write_Char(0x78);
    else Lcd_Write_Char(')');

}

void HomeScreen(unsigned short freq) {
    
    char output[5];
    int decimal, number;
    Lcd_Clear();
    Lcd_Set_Cursor(1,1);
    if ((103 < freq) && (freq < 105)) Lcd_Write_String("BBC Surrey");
    else Lcd_Write_String("Unknown");

    Lcd_Set_Cursor(2, 1);    
    decimal = freq % 10;
    number = freq/10 ;  
    sprintf(output, "%u.%u MHZ ", number, decimal);      
    Lcd_Write_String(output);
    DisplayMuteSymbol(hardmute);
   
    
        
    
    
}

void SeekScreen(char direction)
{
    Lcd_Clear();
    Lcd_Set_Cursor(1,1);
    if (direction == 'u')Lcd_Write_String("Seeking Up");
    if (direction == 'd')Lcd_Write_String("Seeking Down");
    
    
}

void VolumeScreen (int level) 
{
    /*T1CONbits.TMR1ON = 0;
    TMR1L = 0x00;
    TMR1H = 0x00;
    T0CONbits.TMR0ON = 0;*/
    Lcd_Clear();
    delaytime = 0;
    
    
    //Lcd_Clear();
    int count;
    char output[5];
    
    Lcd_Set_Cursor(1, 1);
    sprintf(output, "Volume: %u ", level);
    Lcd_Write_String(output);    
    level = level - 2;
    Lcd_Set_Cursor(2, 1);
    for (count = 0; count < level; count++) {

        Lcd_Write_Char('ÿ');
    }

    for (count; count < 17; count++) {

        Lcd_Write_Char(' ');
    }
  
    
    
   
    
   
   
    
}


