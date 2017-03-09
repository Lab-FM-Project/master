//#include <EEPROM.h>

/* Alex Young Semester 2 2016/17
   Implementing WiFi connectivity and MQTT control for the radio receiver project.
 */

 /* cloudmqtt.com account details:
  mqtt email: alex's email
  company name: FM_Project
  password: jnds43-8_d
  instance: MQTT_Remote_Broker
*/

// including some headers that will be used for the receiver and esp chip
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
///#include "gpio.h"
//#include <pins_nodeMCU.h>
//#include <MQTT.h>
/*#define D0 16
    #define D1 5
    #define D2 4
    #define D3 0
    #define D4 2
    #define D5 14
    #define D6 12
    #define D7 13 */
#define volUpPin D1
#define volDownPin D2
#define stationNextPin D6
#define stationPrevPin D3
#define mutePin D0
#define fav1Pin D5
#define fav2Pin D7
#define fav3Pin D4

#define msgDelay 2000

/*#define user twdgskxg
#define password vkMS2TFzBxLc
#define sslport 28296 */

/*const char *ssid =  "AndroidAP";    // cannot be longer than 32 characters!
const char *password =  "xhct2880";  // insert your internet SSID and password */

/*const char *ssid = "Landgate";
const char *password = "GlaziersLand2EDgateLaneGU3"; */
const char *ssid = "G5_8814";
const char *password = "Beano1234";
const int output = 4; // output that will drive the PIC high or low
const char* mqtt_server = "m21.cloudmqtt.com";
const char* clientID = "ESP8266Client";
const char* clientUserName = clientID;
const char* clientPassword = "mqtt";
int port = 18296;


WiFiClient espClient; //initialising new WifiClient
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
char recvMsg[1];

void setup() { //in the setup we initialise the RC switch to a specific pin (12) and setting output as output pin.
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  digitalWrite(BUILTIN_LED, HIGH); //off
   pinMode(volUpPin, OUTPUT);
  digitalWrite(volUpPin, HIGH);
   pinMode(volDownPin, OUTPUT);
  digitalWrite(volDownPin, HIGH);
  pinMode(stationNextPin, OUTPUT);
  digitalWrite(stationNextPin, HIGH);
   pinMode(stationPrevPin, OUTPUT);
  digitalWrite(stationPrevPin, HIGH);
  pinMode(mutePin, OUTPUT);
  digitalWrite(mutePin, HIGH);
   pinMode(fav1Pin, OUTPUT);
  digitalWrite(fav1Pin, HIGH);
  pinMode(fav2Pin, OUTPUT);
  digitalWrite(fav2Pin, HIGH);
   pinMode(fav3Pin, OUTPUT);
  digitalWrite(fav3Pin, HIGH);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, port);
  client.setCallback(callback);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("callback function called");
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println((char)payload[0]);
  String topicString;
  int sLen = strlen(topic);
  for(int s=0; s<sLen; s++){
    topicString += topic[s];
  } 
  if (topicString == "volume" && (char)payload[0] == 'u'){
    digitalWrite(volUpPin, LOW);
    digitalWrite(BUILTIN_LED, LOW);
    delay(msgDelay);
    digitalWrite(volUpPin, HIGH);
    digitalWrite(BUILTIN_LED, HIGH);
  }
  if (topicString == "volume" && (char)payload[0] == 'd'){
    digitalWrite(volDownPin, LOW);
    digitalWrite(BUILTIN_LED, LOW);
    delay(msgDelay);
    digitalWrite(volDownPin, HIGH);
    digitalWrite(BUILTIN_LED, HIGH);
  }
  if (topicString == "station" && (char)payload[0] == 'u'){
    digitalWrite(stationNextPin, LOW);
    digitalWrite(BUILTIN_LED, LOW);
    delay(msgDelay);
    digitalWrite(stationNextPin, HIGH);
    digitalWrite(BUILTIN_LED, HIGH);
  }
  if (topicString == "station" && (char)payload[0] == 'd'){
    digitalWrite(stationPrevPin, LOW);
    digitalWrite(BUILTIN_LED, LOW);
    delay(msgDelay);
    digitalWrite(stationPrevPin, HIGH);
    digitalWrite(BUILTIN_LED, HIGH);
  }
  if (topicString == "mute" && (char)payload[0] == 'u'){
    digitalWrite(mutePin, LOW);
    digitalWrite(BUILTIN_LED, LOW);
    delay(msgDelay);
    digitalWrite(mutePin, HIGH);
    digitalWrite(BUILTIN_LED, HIGH);
  }
  if (topicString == "favourite" && (char)payload[0] == '1'){
    digitalWrite(fav1Pin, LOW);
    digitalWrite(BUILTIN_LED, LOW);
    delay(msgDelay);
    digitalWrite(fav1Pin, HIGH);
    digitalWrite(BUILTIN_LED, HIGH);
  }
  if (topicString == "favourite" && (char)payload[0] == '2'){
    digitalWrite(fav2Pin, LOW);
    digitalWrite(BUILTIN_LED, LOW);
    delay(msgDelay);
    digitalWrite(fav2Pin, HIGH);
    digitalWrite(BUILTIN_LED, HIGH);
  }
  if (topicString == "favourite" && (char)payload[0] == '3'){
    digitalWrite(fav3Pin, LOW);
    digitalWrite(BUILTIN_LED, LOW);
    delay(msgDelay);
    digitalWrite(fav3Pin, HIGH);
    digitalWrite(BUILTIN_LED, HIGH);
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientID, clientUserName, clientPassword)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("remote_rcv", "hello world");
      // ... and resubscribe
      client.subscribe("volume");
      client.subscribe("station");
      client.subscribe("mute");
      client.subscribe("favourite");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
 //   snprintf (msg, 75, "hello world #%ld", value);
 //   Serial.print("Publish message: ");
  //  Serial.println(msg);
 //   client.publish("remote_rcv", msg);
  }
}

