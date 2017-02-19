
/*
  MQTT subscriber example

  - connects to an MQTT server
  - subscribes to the topic "inTopic"
*/

const byte ledPin = 14;
const byte interruptPin = 2;
volatile byte state = LOW;

// Variables will change:
int ledState = HIGH;         // the current state of the output pin
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin

// the following variables are unsigned long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <MQTT.h>

const char *ssid =  "BTHub6-GP2X";    // cannot be longer than 32 characters!
const char *pass =  "TLdRFAHymPW6";   //

#define BUFFER_SIZE 100

void callback(const MQTT::Publish& pub) {
  Serial.println(pub.payload_string());
  if (pub.payload_string() == "1") {
    digitalWrite(ledPin, HIGH);
  }
  else {
    digitalWrite(ledPin, LOW);
  }
}

WiFiClient wclient;
PubSubClient client(wclient, "m21.cloudmqtt.com", 18694);

void setup() {
  // Setup console
  Serial.begin(115200);
  delay(10);
  Serial.println("HELLO");
  Serial.println();
  
  pinMode(ledPin, OUTPUT);
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), buttonChange, CHANGE);
  // set initial LED state
  digitalWrite(ledPin, state);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.printf("Connecting to ");
    Serial.printf(ssid);
    Serial.println("...");
    WiFi.begin(ssid, pass);

    if (WiFi.waitForConnectResult() != WL_CONNECTED)
      return;
    Serial.println("WiFi connected");
  }

  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      if (client.connect(MQTT::Connect("arduinoClient")
                         .set_auth("scknjdou", "d44gkmL5YXx5"))) {
        Serial.println("Connected to MQTT server");
        client.set_callback(callback);
        client.subscribe("test/Topic");
      }
    }

    
    if (client.connected())
      client.loop();
  }

}

void buttonChange() {
  state = !state;
  client.publish("test/Topic", String(state));
  digitalWrite(ledPin, state);

}

