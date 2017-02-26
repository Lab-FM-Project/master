
//LCD Functions Developed by electroSome

#include <stdio.h>
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
void display_mute_symbol();
void HomeScreen(float freq);
void VolumeScreen(int level);


void Lcd_Port(char a)
{
	if(a & 1)
		D4 = 1;
	else
		D4 = 0;

	if(a & 2)
		D5 = 1;
	else
		D5 = 0;

	if(a & 4)
		D6 = 1;
	else
		D6 = 0;

	if(a & 8)
		D7 = 1;
	else
		D7 = 0;
}
void Lcd_Cmd(char a)
{
	RS = 0;             // => RS = 0
	Lcd_Port(a);
	EN  = 1;             // => E = 1
        __delay_ms(4);
        EN  = 0;             // => E = 0
}

void Lcd_Clear()
{
	Lcd_Cmd(0);
	Lcd_Cmd(1);
}

void Lcd_Set_Cursor(char a, char b)
{
	char temp,z,y;
	if(a == 1)
	{
	  temp = 0x80 + b - 1;
		z = temp>>4;
		y = temp & 0x0F;
		Lcd_Cmd(z);
		Lcd_Cmd(y);
	}
	else if(a == 2)
	{
		temp = 0xC0 + b - 1;
		z = temp>>4;
		y = temp & 0x0F;
		Lcd_Cmd(z);
		Lcd_Cmd(y);
	}
}
   

void Store_mute_symbol()

{   
    
    Lcd_Cmd(0x40); // set the address to CGRAM
    Lcd_Write_Char(0x00);
    Lcd_Write_Char(0x02);
    Lcd_Write_Char(0x06);
    Lcd_Write_Char(0x0E);
    Lcd_Write_Char(0x0E);
    Lcd_Write_Char(0x06);
    Lcd_Write_Char(0x02);
    Lcd_Write_Char(0x00);
    
    
}
void Lcd_Init()
{
  Lcd_Port(0x00);
   __delay_ms(20);
  Lcd_Cmd(0x03);
	__delay_ms(5);
  Lcd_Cmd(0x03);
	__delay_ms(11);
  Lcd_Cmd(0x03);
  /////////////////////////////////////////////////////
  Lcd_Cmd(0x02);
  Lcd_Cmd(0x02);
  Lcd_Cmd(0x08);
  Lcd_Cmd(0x00);
  Lcd_Cmd(0x0C);
  Lcd_Cmd(0x00);
  Lcd_Cmd(0x06);
  //Store_mute_symbol();
  
}

void Lcd_Write_Char(char a)
{
   char temp,y;
   temp = a&0x0F;
   y = a&0xF0;
   RS = 1;             // => RS = 1
   Lcd_Port(y>>4);             //Data transfer
   EN = 1;
   __delay_us(40);
   EN = 0;
   Lcd_Port(temp);
   EN = 1;
   __delay_us(40);
   EN = 0;
}

void Lcd_Write_String(char *a)
{
	int i;
	for(i=0;a[i]!='\0';i++)
	   Lcd_Write_Char(a[i]);
}

void Lcd_Shift_Right()
{
	Lcd_Cmd(0x01);
	Lcd_Cmd(0x0C);
}

void Lcd_Shift_Left()
{
	Lcd_Cmd(0x01);
	Lcd_Cmd(0x08);
}



void display_mute_symbol()
{
    Lcd_Set_Cursor(1,15);
    Lcd_Write_Char(0x00);
    
}


void HomeScreen(float freq)
{
	 
    Lcd_Set_Cursor(1,1);
    if ((103.0 < freq) && (freq <105.0)) Lcd_Write_String("BBC Surrey");
    else Lcd_Write_String("Unknown Station");    
           
    Lcd_Set_Cursor(2,1);
    char output[5];
    sprintf(output, "%.1f MHZ", freq);   
    Lcd_Write_String(output);
   
}

void VolumeScreen(int level)
{
	int count;
    level = level - 2;    
    Lcd_Set_Cursor(1,1);
    Lcd_Write_String("Volume: ");
    Lcd_Set_Cursor(2,1);
    for (count = 0; count < level; count++)   
    {
        
        Lcd_Write_String("ÿ");
    }
    
     for (count; count < 17; count++)   
    {
        
        Lcd_Write_String(" ");
    }

    
    
    
   
}
