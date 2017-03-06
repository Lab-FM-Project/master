/*
-----===== Arduino Web radio player ====----

(c)05 October 2014 by Vassilis Serasidis

Home: http://www.serasidis.gr
email: avrsite@yahoo.gr
Version: v1.0

************************************************************************************************
**************** Current webradio player is modified by William Dudley *************************
    Additional features:
	- 16x2 LCD is used
	- Analog A0 pin buttons are used  (See LCDKeypad library)
	- Last selected radio station is stored in eeprom for retrieving back after next power ON.  
	
	This version uses different Arduino pin order. 
	Arduino     VS1053  
   --------     -------  	
	  A5  	  	cs_pin
	  A2  		dcs_pin
	  A3  		dreq_pin
	  A4  		reset_pin
	
	Arduino     LCD 2x16
	---------  ----------
	  8			  RS
	  GND		  R/W
	  9			  E
	  4           DB4
	  5           DB5
	  6           DB6
	  7           DB7
	  
	  The schematic for LCD keypad can be found here:
	  http://www.dfrobot.com/wiki/images/a/a7/LCDKeypad_Shield_SCH.png
************************************************************************************************


Hardware:
- Any Arduino (I prefer Arduino pro mini or nano because of its small size and price).
- ENC28J60 ethernet module  (EtherCard library is used   - Written by Jean-Claude Wippler)
- VS1053B MP3 module/shield (VS1053B library is used     - Written by J. Coliz)
- Nokia 5110 LCD module     (PCD8544_mod library is used - Written by Carlos Rodrigues - Modified by Vassilis Serasidis )

Additional libraries:
- TimerOne  (Software watchdog timer - Originally written by Jesse Tane, last modification Oct 2011 by Andrew Richards).
- LCDKeypad (5 buttons connected on PC0 analog pin).

Description:
- This web radio player has 14 pre-defined web radio stations (station1, station2,...,station13, station14).
You can add your favorite webradio stations. There is much free available flash memory.

- For more web radio stations visit http://www.internet-radio.com/
Visit the http://www.internet-radio.com/ , choose the stations with up to 64-kbps and copy the IP and the Port of the web radio.
For example, right-click on your favorite 64-kbps webradio icon and select <save link location. Then, paste it to a text file. 
You will see some think like that:->  http://servers.internet-radio.com/tools/playlistgenerator/?u=http://108.163.215.90:8006/listen.pls&t=.pls
Replace the current station "station1_IP[] = {205,164,36,153}" with the new station "station1_IP[] = {108,163,215,90}".
Do the same to the port. Replace the "station1_Port = 80" with the new one "station1_Port = 8006".

You can replace the whole 14 stations with the stations of your choice.

** This circuit doesn't use a big RAM buffer. That causes a small lag (de-synchronize) due to delay transmission of data packets from the webradio server to your webradio player.

** I will try to improve the source code for supporting 128-kbps web radio stations.


 ***** This source code and hardware are provided as is without any warranty under GNU GPL v3 licence *****
     
     Have a good listening!
*/

#include <avr/pgmspace.h>
//#include <EtherCard.h> //want
#include <VS1053.h>
#include <SPI.h>
#include <Wire.h>
//#include <LiquidCrystal.h>
//#include <LCDKeypad.h>
#include <TimerOne.h>

#define BUFFER_LENGTH 600       //Ethernet data buffer length.
#define BUFFER_LENGTH2 32       //VS1053 data buffer length
// #define LED1    A1

boolean ViewStationInfo = false;
byte Ethernet::buffer[BUFFER_LENGTH];
byte MP3_buffer[BUFFER_LENGTH2];
int indexCounter = 0;
static uint32_t timer;

int radioStationNumber = 1; //Initial webradio station
boolean radioStationIsChanged = false;
boolean receivedData = true;

#define ETH_CS A1
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0xF9 };
static byte myip[]  = { 192,168,27,230 }; //manually sets the IP, Gateway IP and DNS in case DHCP fails.
static byte gwip[]  = { 192,168,27,1 };
static byte dnsip[] = {   8,  8, 4,4 };

// VS1053  player(9, 6, 7, 8);  // cs_pin, dcs_pin, dreq_pin, reset_pin
VS1053  player(A5, A2, A3, A4);  // cs_pin, dcs_pin, dreq_pin, reset_pin
//LCDKeypad lcd;
// which does this:
// LCDKeypad::LCDKeypad() : LiquidCrystal(8, 9, 4, 5, 6, 7)

typedef struct station {
    byte ip[4];
    unsigned int port;
} STATION;

STATION stations[] PROGMEM = {
	{ {205,164, 36,153},    80 },	//BOM PSYTRANCE (1.FM TM)  64-kbps
	{ {205,164, 62, 15}, 10032 },	//1.FM - GAIA, 64-kbps
	{ {109,206, 96, 11},    80 },	//TOP FM Beograd 106,8  64-kpbs
	{ { 85, 17,121,216},  8468 },	//RADIO LEHOVO 971 GREECE, 64-kbps
	{ { 85, 17,121,103},  8800 },	//STAR FM 88.8 Corfu Greece, 64-kbps
	// 6
	{ { 85, 17,122, 39},  8530 },	//www.stylfm.gr laiko, 64-kbps
	{ {144, 76,204,149},  9940 },	// RADIO KARDOYLA - 64-kbps 22050 Hz
	{ {198, 50,101,130},  8245 },	//La Hit Radio, Rock - Metal - Hard Rock, 32-kbps
	{ { 94, 23, 66,155},  8106 },	// *ILR CHILL & GROOVE* 64-kbps
	{ {205,164, 62, 22},  7012 },	//1.FM - ABSOLUTE TRANCE (EURO) RADIO   64-kbps
	// 11
	{ {205,164, 62, 13}, 10144 },	//1.FM - Sax4Ever   64-kbps 
	{ { 83,170,104, 91}, 31265 },	//Paradise Radio 106   64-kbps
	{ {205,164, 62, 13}, 10152 },	//Costa Del Mar - Chillout (1.FM), 64-kbps
	{ { 46, 28, 48,140},  9998 }, 	//AutoDJ, latin, cumbia, salsa, merengue, regueton, pasillos , 48-kbps
	{ { 50,  7,173,162},  8116 }, 	// Big B Radio #CPOP - Asian Music - 64k
	// 16
	{ { 50,  7,173,162},  8097 }, 	// Big B Radio #AsianPop - 64kbps
	{ {195,154,167, 62},  7264 }, 	// Radio China - 48kbps
	{ {198,154,106,104},  8985 } 	// radioICAST.com Acid Jazz Blues Rock 96K
    };

#define LAST_STATION_NUMBER (sizeof(stations)/sizeof(STATION))  //The last station

//============================================================================
// called when the client request is complete
//============================================================================
static void my_callback (byte status, word off, word len) {
    unsigned int i;
    char *cp1, *cp2, lbuf[33];
    int sp; //lcd_title_found;
    byte j;
   // lcd_title_found = 0;

    
    if((indexCounter < 500) && (ViewStationInfo == false)) {
   /*    if(!lcd_title_found) {
	   lcd.clear();
	   lcd.setCursor(0, 0); //LCD line 0
       }*/
       if((cp1 = strstr_P((const char *)&Ethernet::buffer[off], PSTR("icy-name"))) != 0) {
	    Serial.println(F("strstr found 'icy-name'"));
	    cp1 += 9;
	    strncpy(lbuf, cp1, 32);
	    if((cp2 = strstr_P((const char *)lbuf, PSTR("\r"))) != 0) {
		*cp2 = '\0';
		j = strlen(lbuf);
		cp1 += j + 12; 
		strncpy(cp2, cp1, 32 - j);
	    }
	    if((cp2 = strstr_P((const char *)lbuf, PSTR("\r"))) != 0) {
		*cp2 = '\0';
	    }
	  /*  for(j = 0 ; j < 16 && lbuf[j] ; j++) {
		  lcd.write(lbuf[j]);
	    }
	   lcd.setCursor(0, 1); //LCD line 1
	    for( ; j < 32 && lbuf[j] ; j++) {
	    	lcd.write(lbuf[j]);
	    }
	    lcd_title_found = 1;
       } */
    /*   for(i=0;i<len;i++) {
	   Serial.write(Ethernet::buffer[off+i]); //Show the web radio channel information (name, genre, bit rate etc).
	   if(!lcd_title_found) {
	       if((indexCounter + i > 179) && (indexCounter + i < 301)) 
		   if(strncmp_P((const char *)&Ethernet::buffer[off + i], PSTR("icy-name"), 8) == 0) {
		   Serial.println(F("strcmp found 'icy-name'"));
		   for(byte j = 0 ; j < 16 ; j++) {
		       lcd.write(Ethernet::buffer[off + i + j + 8]);
		   }
	       }
	       else {
		   // for(byte j = 0 ; j < 16 ; j++) { }
		   lcd.write(Ethernet::buffer[off+i]);
	       } */
	   }
	   
	   if(Ethernet::buffer[off+i] == 0x0d && 
	      Ethernet::buffer[off+i+1] == 0x0a && 
	      Ethernet::buffer[off+i+2] == 0x0d && 
	      Ethernet::buffer[off+i+3] == 0x0a)
	      {
		  ViewStationInfo = true;
		  break; //We found the index end (0x0d,0x0a,0x0d,0x0a). Do not search anymore.
	   }
	}
      indexCounter += len;
    }
    else {
       uint8_t* data = (uint8_t *) Ethernet::buffer + off; //Get the data stream from ENC28J60 and...
       player.playChunk(data, len);                        //...send them to VS1053B
       timer = millis();                                   //Update the timeout timer.
       receivedData = true;
    }          
}

//===========================================================================
//
//===========================================================================

//want this
void setup() {
        // pinMode(LED1, OUTPUT); //LED1 is connected to A1 pin 
        
	Serial.begin(57600);    //Start serial port with 57600 bits per seconds
	SPI.begin();            //Start Serial Peripheral Interface (SPI)
	player.begin();         //Start VS1053B
	player.setVolume(0);    //Set the volume to the maximum.
        
     /*   lcd.begin(16, 2); //2 rows x 16 chars text
        lcd.clear();
        lcd.setCursor(0, 0);	// column, row
        lcd.print(F("WEBRADIO PLAYER")); */
        delay(5000);
        
        
	Serial.println(F("\n--== Arduino WebRadio player ==--\n"));
        Serial.println(F("(c)2014 by Vassilis Serasidis"));
        Serial.println(F("http://www.serasidis.gr\n"));
        Serial.println(F("Connecting..."));

	//Initiallize ENC28J60. Chip Select pin (CS) is on arduino pin D10.
	while (ether.begin(sizeof Ethernet::buffer, mymac, ETH_CS) == 0) {
	  Serial.println(F("Failed to access Ethernet controller"));
	//  lcd.setCursor(0, 1);	// column, row
	//  lcd.print(F("ether failed"));
	  delay(10);
	}
//	lcd.setCursor(0, 1);	// column, row
//	lcd.print(F("ethernet OK "));
	Serial.println(F("got past ether.begin()"));
	while (!ether.dhcpSetup()) {
	  Serial.println(F("DHCP failed"));
//	  lcd.setCursor(0, 1);	// column, row
//	  lcd.print(F("DHCP failed"));
	  delay(10);
	}
	Serial.println(F("got past dhcpSetup()"));
//	lcd.setCursor(0, 1);	// column, row
//	lcd.print(F("DHCP ok     "));

        ether.persistTcpConnection(true); //Keep TCP/IP connection alive. 
        ViewStationInfo = true;
        
        Timer1.initialize(5000000); // set a timer of length 5000000 microseconds (or 5 sec => the led will blink 5 times, 5 cycles of on-and-off, per second)
        Timer1.attachInterrupt( timerIsr ); // attach the service routine here
}

//=============================================================================
//
//=============================================================================

//want this
void loop()
{
int buttonPressed;
   ether.packetLoop(ether.packetReceive()); //Read ENC28J60 for new incoming data packet.

   buttonPressed=lcd.button();
   //Check if the 'next' switch has been pressed.
   if(buttonPressed == KEYPAD_RIGHT) {
     while(buttonPressed == KEYPAD_RIGHT) {
       delay(100);
       buttonPressed=lcd.button();
     }
     if(radioStationNumber < LAST_STATION_NUMBER) {
       radioStationNumber += 1;
       radioStationIsChanged = true;
     }
     else {
       radioStationNumber = 1;
       radioStationIsChanged = true;
     }
   }
   
   if(buttonPressed == KEYPAD_UP) {
     while(buttonPressed == KEYPAD_UP) {
       delay(100);
       buttonPressed=lcd.button();
     }
     radioStationNumber += 5;
     radioStationIsChanged = true;
     if(radioStationNumber > LAST_STATION_NUMBER) {
       radioStationNumber -= LAST_STATION_NUMBER ;
     }
   }
   //Check if the 'previous' switch has been pressed.
   if(buttonPressed== KEYPAD_LEFT) {
       while(buttonPressed == KEYPAD_LEFT) {
         delay(100);
         buttonPressed=lcd.button();
       }
       if(radioStationNumber > 1) {
         radioStationNumber -= 1;
         radioStationIsChanged = true;
       }
       else {
	 radioStationNumber = LAST_STATION_NUMBER;
	 radioStationIsChanged = true;
       }
   }
   if(buttonPressed == KEYPAD_DOWN) {
     while(buttonPressed == KEYPAD_DOWN) {
       delay(100);
       buttonPressed=lcd.button();
     }
     radioStationNumber -= 5;
     radioStationIsChanged = true;
     if(radioStationNumber < 1) {
       radioStationNumber += LAST_STATION_NUMBER;
     }
   }
   
   //If 'next' or 'previous' switch has been pressed, play the selected webradio station.
   if(radioStationIsChanged == true) {
     char pnum[4];
     STATION stemp;
     sprintf(pnum, "%2d", radioStationNumber);
     // copy STATION data from PROGMEM to stemp in ram:
     for (unsigned char k = 0; k < 4; k++) {
	 stemp.ip[k] = pgm_read_byte(&(stations[radioStationNumber-1].ip[k]));
     }
     stemp.port = pgm_read_word(&(stations[radioStationNumber-1].port));
     playWebRadioStation(stemp.ip, stemp.port, pnum);
   }
  
   radioStationIsChanged = false; 
   
  if((millis() > timer + 5000)) // Timeout timer. If the song stops playing for 5 seconds re-connect to the server.
  {   
   radioStationIsChanged = true; 
   ViewStationInfo = false;
   Serial.print(F("\nre-connecting to the server...\n"));
  /* lcd.clear();
   lcd.setCursor(0,0);
   lcd.print(F("re-connecting to"));
   lcd.setCursor(0,1);
   lcd.print(F("the server")); */
   timer = millis(); 
  }
}

//============================================================================
//
//============================================================================

//want this
void playWebRadioStation ( byte ip[4], const int hisPort, char* preset )
{
  Serial.print("\n\n<"); //Print the station info to the serial port
  Serial.print(preset);
  Serial.println(F("> ============================================================"));
  
  player.stopSong();
  ViewStationInfo = false;
  indexCounter = 0;
  ether.copyIp(ether.hisip, ip);
  ether.hisport = hisPort;
  ether.printIp("IP:   ", ether.myip);
  ether.printIp("GW:   ", ether.gwip);
  ether.printIp("DNS:  ", ether.dnsip);
  ether.printIp(F("SRV:  "), ether.hisip); 
  Serial.print("Port: ");
  Serial.println(ether.hisport);
  Serial.println();
  //ether.browseUrl(PSTR("/"), "",PSTR(""), PSTR("Icy-MetaData:1"), my_callback);
  ether.browseUrl(PSTR("/"), "",PSTR(""), PSTR(""), my_callback);
 /* lcd.clear();
  lcd.setCursor(0, 0); //LCD line 0
  lcd.write('<'); 
  lcd.print(preset); //Show on LCD the webradio number (1-14)
  lcd.write('>');
  lcd.write(' '); */
}

//============================================================================
// Custom ISR Timer Routine
//============================================================================
void timerIsr() {
    // Toggle LED
    // digitalWrite( LED1, digitalRead( LED1 ) ^ 1 );
    if(receivedData == true) {
      receivedData = false;
    }
    else {
      radioStationIsChanged = true; 
      if (ether.begin(sizeof Ethernet::buffer, mymac, ETH_CS) == 0) //Initialize ENC28J60. Chip Select pin (CS) is connected on arduino pin D10.
	  Serial.println(F("Failed to access Ethernet controller"));
      if (!ether.dhcpSetup())
	  Serial.println(F("DHCP failed"));
      ether.persistTcpConnection(true); //Keep TCP/IP connection alive.
    }      
}

