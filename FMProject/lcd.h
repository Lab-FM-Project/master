#include <stdio.h>
#define RS PORTDbits.RD2
#define EN PORTDbits.RD3
#define D4 PORTDbits.RD4
#define D5 PORTDbits.RD5
#define D6 PORTDbits.RD6
#define D7 PORTDbits.RD7
#define TIMER_RESET_VALUE 6 
/*Calculated by the formula
//TMR0 = 256 - (timerPeriod*Fosc)/(4*prescaler) + x

//In this case, timerPeriod = 0.001s
//              Fosc = 8,000,000
//              prescaler = 8
//              x = 0 because PS is > 2

//So,   TMR1 = 256 - (0.001*8000000)/(4*8)
//      TMR1 = 256 - 8000/16
//      TMR1 = 6 */

void Store_mute_symbol();
void Lcd_Port(char a);
void Lcd_Cmd(char a);
void Lcd_Clear();
void Lcd_Set_Cursor(char a, char b);
void Lcd_Init();
void Lcd_Write_Char(char a);
void Lcd_Write_String(char *a);
void DisplayMuteSymbol(unsigned char muted);
void HomeScreen(unsigned short freq);
void VolumeScreen (int level);
void Lcd_Write(char * Text, unsigned char CursorRow, unsigned char CursorCol, unsigned char Clear);

// writes a character to the screen
void Lcd_Write_Char(char a) {
    char temp,y;
   temp = a&0x0F; // temp masked with second nibble of ASCII code
   y = a&0xF0;// y masked with first nibble of ASCII code
   // send first nibble
   RS = 1;        
   Lcd_Port(y>>4); 
   EN = 1;
   __delay_us(60);
   // send second nibble
   EN = 0;
   Lcd_Port(temp); 
   EN = 1;
   __delay_us(60);
    EN = 0;
}
// send nibble on data lines
void Lcd_Port(char a) 
{
    if (a & 1) D4 = 1;
    else D4 = 0;
    
    if (a & 2) D5 = 1;
    else D5 = 0;
    
    if (a & 4) D6 = 1;
    else D6 = 0;

    if (a & 8) D7 = 1;
    else D7 = 0;
}

// sends command to LCD screen
void Lcd_Cmd(char a) 
{
    RS = 0;             // => RS = 0
	Lcd_Port(a);
	EN  = 1;             // => E = 1
    __delay_ms(4);
    EN  = 0;             // => E = 0
}
// clear LCD Screen
void Lcd_Clear() 
{
    Lcd_Cmd(0);
    Lcd_Cmd(1);
}
// moves cursor on LCD
void Lcd_Set_Cursor(char a, char b) 
{
    char temp, z, y;
    if (a == 1) 
    {
        temp = 0x80 + b - 1;
        z = temp >> 4;
        y = temp & 0x0F;
        Lcd_Cmd(z);
        Lcd_Cmd(y);
    } 
    else if (a == 2) 
    {
        temp = 0xC0 + b - 1;
        z = temp >> 4;
        y = temp & 0x0F;
        Lcd_Cmd(z);
        Lcd_Cmd(y);
    }
}

void Store_mute_symbol()
 {
    // set the address to CGRAM location 0
    Lcd_Cmd(0x04);
    Lcd_Cmd(0x00); 
    // character code
    Lcd_Write_Char(0x02); 
    Lcd_Write_Char(0x06); 
    Lcd_Write_Char(0x1e); 
    Lcd_Write_Char(0x1e);
    Lcd_Write_Char(0x1e);
    Lcd_Write_Char(0x1e);
    Lcd_Write_Char(0x06);
    Lcd_Write_Char(0x02);
}

void Lcd_Init() 
{
  TRISD = 0x00;// set PortD to all outputs for LCD
  // intialise LCD
  Lcd_Port(0x00);
   __delay_ms(40);
  Lcd_Cmd(0x03);
	__delay_ms(10);
  Lcd_Cmd(0x03);
	__delay_ms(22);
  Lcd_Cmd(0x03);
  Lcd_Cmd(0x02);
  Lcd_Cmd(0x02);
  Lcd_Cmd(0x08);
  Lcd_Cmd(0x00);
  Lcd_Cmd(0x0C);
  Lcd_Cmd(0x00);
  Lcd_Cmd(0x06);
      
  Store_mute_symbol(); // put mute symbole in GGRAM
  Lcd_Clear();    
}

//write string
void Lcd_Write_String(char *a) 
{
    int i;
	for(i=0;a[i]!='\0';i++)
	   Lcd_Write_Char(a[i]);
}

//sets cursors, clears the screen if clear = 1, and then writes input string to LCD
void Lcd_Write(char * Text, unsigned char CursorRow, unsigned char CursorCol, unsigned char Clear) 
{
    if (Clear == 1) Lcd_Clear();
    Lcd_Set_Cursor(CursorRow, CursorCol);
    Lcd_Write_String(Text);
}


//Displays Customs Character
void DisplayMuteSymbol(unsigned char muted) 
{
    Lcd_Set_Cursor(1, 15);// set cursor to top right
    Lcd_Write_Char(0x00); // fill DDRAM with address of our custom character
    Lcd_Set_Cursor(1, 16); // move cursor along one segment
    // add ')' or 'x' depending on mute state
    if (muted == 1) Lcd_Write_Char(0x78); 
    else Lcd_Write_Char(')');

}

void HomeScreen(unsigned short freq) {
    
    char output[16] = "                ";
    unsigned short decimal, number;
    /*write channel on screen*/
    switch (freq/10)
    {
        case 104: Lcd_Write("BBC Surrey", 1,1,1); break;
        case 96: Lcd_Write("Eagle Radio", 1,1,1); break;
        case 97: Lcd_Write("Radio 1", 1,1,1); break;
        case 88: Lcd_Write("Radio 2", 1,1,1); break;
        case 90: Lcd_Write("Radio 3", 1,1,1); break;
        case 92: Lcd_Write("Radio 4", 1,1,1); break;
        case 103: Lcd_Write("Kane FM", 1,1,1); break;
        default:  Lcd_Write("Unknown", 1,1,1);
    }      
    /* convert frequency to display float*/
    decimal = freq % 10;
    number = freq/10;  
    sprintf(output, "%u.%u MHZ ", number, decimal);      
    Lcd_Write(output, 2,1,0);
    DisplayMuteSymbol(hardmute);  // update mute symbol  
}



void VolumeScreen (int level) 
{  
    int count;
    char output[16];
    delaytime = 0; // rest volume screen timer 
    
    sprintf(output, "Volume: %u ", level);
    Lcd_Write(output, 1,1,1);    
    level = level - 2; // so that the segments move right
    /*add level segments */
    Lcd_Set_Cursor(2, 1);
    for (count = 0; count < level; count++) Lcd_Write_Char('ÿ');    
    /* clear old segments*/
    for (count; count < 17; count++) Lcd_Write_Char(' ');    
}


