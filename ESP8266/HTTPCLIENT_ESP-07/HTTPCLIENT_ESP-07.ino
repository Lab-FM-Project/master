/*
 */

#include <Arduino.h>
#include <DHT.h>;
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

//Constants
#define BUS_ID 2
#define DHTPIN 14     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initialize DHT sensor for normal 16mhz Arduino
#define USE_SERIAL Serial

//Variables
int chk;
float hum;  //Stores humidity value
float temp; //Stores temperature value

ESP8266WiFiMulti WiFiMulti;

void setup() {

    
    USE_SERIAL.begin(115200);
   // USE_SERIAL.setDebugOutput(true);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

    for(uint8_t t = 4; t > 0; t--) {
        USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }

    WiFiMulti.addAP("Junction", "");
    dht.begin();


}

void loop() {

        hum = random(40,60);
        temp= random(20, 30);
        //Print temp and humidity values to serial monitor
        Serial.print("Humidity: ");
        Serial.print(hum);
        Serial.print(" %, Temp: ");
        Serial.print(temp);
        Serial.println(" Celsius");
        delay(1000);
    // wait for WiFi connection
    if((WiFiMulti.run() == WL_CONNECTED)) {

        HTTPClient http;

        USE_SERIAL.print("[HTTP] begin...\n");
  
        String humidity = "http://85.188.10.135:8000/sensor?stype=humidity";
        humidity +="&svalue=";
        humidity += hum;
        humidity +="&busid=";
        humidity += BUS_ID;
        humidity +="&rawvalue=3";
        Serial.println(humidity);
        
        USE_SERIAL.print("[HTTP] begin...\n");
        // configure traged server and url
        http.begin(humidity); //HTTP
        USE_SERIAL.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        int httpCode = http.GET();

        // httpCode will be negative on error
        if(httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

            // file found at server
            if(httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                USE_SERIAL.println(payload);
            }
        } else {
            USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();

        String temp = "http://85.188.10.135:8000/sensor?stype=temp";
        temp +="&svalue=";
        temp += temp;
        temp +="&busid=";
        temp += BUS_ID;
        temp +="&rawvalue=3";
        Serial.println(temp);
        USE_SERIAL.print("[HTTP] begin...\n");
        // configure traged server and url
        http.begin(temp); //HTTP
        USE_SERIAL.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        httpCode = http.GET();

        // httpCode will be negative on error
        if(httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

            // file found at server
            if(httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                USE_SERIAL.println(payload);
            }
        } else {
            USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
    }

    delay(10000);
}
