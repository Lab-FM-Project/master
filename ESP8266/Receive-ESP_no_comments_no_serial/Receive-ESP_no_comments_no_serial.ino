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
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "ThingSpeak.h"
#include <ESP8266HTTPClient.h>
#include <TimeLib.h>

#define D0 16
#define D1 5
#define D2 4
#define D4 2
#define D5 14
#define D6 12
#define D7 13
#define volUpPin D1
#define volDownPin D2
#define stationNextPin D6
#define stationPrevPin D4
#define mutePin D5
#define fav1Pin D0
#define fav2Pin D7
#define msgDelay 200

// ThingSpeak Settings
char thingSpeakAddress[] = "api.thingspeak.com";
String thingSpeakAddressString = "api.thingspeak.com";
String thingtweetAPIKey = "2UX3X7IJV3BUW4QF";
unsigned long myChannelNumber = 245982;
const char* readAPIKey = "JXLAXB16N291O4FC";
String talkBackID = "14909";
const int checkTalkBackInterval = 15 * 1000;    // Time interval in milliseconds to check TalkBack (number of seconds * 1000 = interval)
String talkBackAPIKey = "XIAJGWMZ731MXAK7";

const char *ssid = "G5_8814";
const char *password = "Beano1234";
/*
  const char *ssid = "Landgate";
  const char *password = "GlaziersLand2EDgateLaneGU3";*/

const int timeZone = 1; // Central European Time
static const char ntpServerName[] = "us.pool.ntp.org";
WiFiUDP Udp;
unsigned int localPort = 8888; // local port to listen for UDP packets
/*-------- NTP code ----------*/
const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

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
// Variable Setup
long lastConnectionTime = 0;
boolean lastConnected = false;
int failedCounter = 0;

void setup() { //in the setup we initialise the RC switch to a specific pin (12) and setting output as output pin.
  pinMode(volUpPin, OUTPUT);
  digitalWrite(volUpPin, LOW);
  pinMode(volDownPin, OUTPUT);
  digitalWrite(volDownPin, LOW);
  pinMode(stationNextPin, OUTPUT);
  digitalWrite(stationNextPin, LOW);
  pinMode(stationPrevPin, OUTPUT);
  digitalWrite(stationPrevPin, LOW);
  pinMode(mutePin, OUTPUT);
  digitalWrite(mutePin, LOW);
  pinMode(fav1Pin, OUTPUT);
  digitalWrite(fav1Pin, LOW);
  pinMode(fav2Pin, OUTPUT);
  digitalWrite(fav2Pin, LOW);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, port);
  client.setCallback(callback);
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
  setSyncInterval(300);
}

void setup_wifi() {
  delay(1000);
  // We start by connecting to a WiFi network
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
  for (int s = 0; s < sLen; s++) {
    topicString += topic[s];
  }
  if (topicString == "volume" && (char)payload[0] == 'u') {
    digitalWrite(volUpPin, HIGH);
    delay(msgDelay);
    digitalWrite(volUpPin, LOW);
  }
  if (topicString == "volume" && (char)payload[0] == 'd') {
    digitalWrite(volDownPin, HIGH);
    delay(msgDelay);
    digitalWrite(volDownPin, LOW);
  }
  if (topicString == "station" && (char)payload[0] == 'u') {
    digitalWrite(stationNextPin, HIGH);
    delay(msgDelay);
    digitalWrite(stationNextPin, LOW);
  }
  if (topicString == "station" && (char)payload[0] == 'd') {
    digitalWrite(stationPrevPin, HIGH);
    delay(msgDelay);
    digitalWrite(stationPrevPin, LOW);
  }
  if (topicString == "mute" && (char)payload[0] == 'u') {
    digitalWrite(mutePin, HIGH);
    delay(msgDelay);
    digitalWrite(mutePin, LOW);
  }
  if (topicString == "favourite" && (char)payload[0] == '1') {
    digitalWrite(fav1Pin, HIGH);
    delay(msgDelay);
    digitalWrite(fav1Pin, LOW);
  }
  if (topicString == "favourite" && (char)payload[0] == '2') {
    digitalWrite(fav2Pin, HIGH);
    delay(msgDelay);
    digitalWrite(fav2Pin, LOW);
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

void updateTwitterStatus(String tsData) {
  Serial.println("updateTwitterStatus");
  delay(3000);
  if (espClient.connect(thingSpeakAddress, 80)) {
    // Create HTTP POST Data
    tsData = "api_key=" + thingtweetAPIKey + "&status=" + tsData + "#stillbetterthanStagRadio";
    espClient.print("POST /apps/thingtweet/1/statuses/update HTTP/1.1\n");
    espClient.print("Host: api.thingspeak.com\n");
    espClient.print("Connection: close\n");
    espClient.print("Content-Type: application/x-www-form-urlencoded\n");
    espClient.print("Content-Length:");
    espClient.print(tsData.length());
    espClient.print("\n\n");
    espClient.print(tsData);
    lastConnectionTime = millis();
    if (espClient.connected()) {
      Serial.println("Connecting to ThingSpeak...");
      Serial.println();
      failedCounter = 0;
    }
    else {
      failedCounter++;
      Serial.println("1 Connection to ThingSpeak failed (" + String(failedCounter, DEC) + ")");
      Serial.println();
    }
  }
  else {
    failedCounter++;
    Serial.println("2 Connection to ThingSpeak Failed (" + String(failedCounter, DEC) + ")");
    Serial.println();
    lastConnectionTime = millis();
  }
}

String checkTalkBack() {
  // POST https://api.thingspeak.com/talkbacks/14909/commands/execute.json
  HTTPClient client_http;
  String talkBackReturnJSON, command;
  String talkBackURL =  "http://api.thingspeak.com/talkbacks/14909/commands/execute.json";
  client_http.begin(talkBackURL);
  int httpCode = client_http.POST("api_key=" + talkBackAPIKey);
  // httpCode will be negative on error
  //  Serial.print("httpcode = ");
  //  Serial.println(httpCode);
  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    //       Serial.println("[HTTP] GET... code: %d\n", httpCode);
    // file found at server
    if (httpCode == HTTP_CODE_OK) {
      talkBackReturnJSON = client_http.getString();
      Serial.print("talkBackReturnJSON = ");
      Serial.println(talkBackReturnJSON);
      command = getCommand(talkBackReturnJSON);
      //        Serial.print("command = ");
      //        Serial.println(command);
    }
  }
  client_http.end();
  Serial.flush();
  return command;
  delay(1000);
}

void deleteDuplicateFromTalkBackQueue() {
  // POST https://api.thingspeak.com/talkbacks/14909/commands/execute.json
  HTTPClient client_http;
  String talkBackReturnJSON;
  String talkBackURL =  "http://api.thingspeak.com/talkbacks/14909/commands/execute.json";
  client_http.begin(talkBackURL);
  int httpCode = client_http.POST("api_key=" + talkBackAPIKey);
  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    //       Serial.println("[HTTP] GET... code: %d\n", httpCode);
    // file found at server
    if (httpCode == HTTP_CODE_OK) {
      talkBackReturnJSON = client_http.getString();
    }
  }
  client_http.end();
  Serial.flush();
}

String getCommand(String data) {
  int index = 5;
  char separator = '"';
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void reactToTwitterControl(String twitterCommand) {
  Serial.print("command ");
  Serial.print(twitterCommand);
  Serial.println(" executed");
  if (twitterCommand == "volumeup") {
    digitalWrite(volUpPin, HIGH);
    delay(msgDelay);
    digitalWrite(volUpPin, LOW);
  }
  if (twitterCommand == "volumedown") {
    digitalWrite(volDownPin, HIGH);
    delay(msgDelay);
    digitalWrite(volDownPin, LOW);
  }
  if (twitterCommand == "stationup") {
    digitalWrite(stationNextPin, HIGH);
    delay(msgDelay);
    digitalWrite(stationNextPin, LOW);
  }
  if (twitterCommand == "stationdown") {
    digitalWrite(stationPrevPin, HIGH);
    delay(msgDelay);
    digitalWrite(stationPrevPin, LOW);
  }
  if (twitterCommand == "mute") {
    digitalWrite(mutePin, HIGH);
    delay(msgDelay);
    digitalWrite(mutePin, LOW);
  }
  if (twitterCommand == "favourite1") {
    digitalWrite(fav1Pin, HIGH);
    delay(msgDelay);
    digitalWrite(fav1Pin, LOW);
  }
  if (twitterCommand == "favourite2") {
    digitalWrite(fav2Pin, HIGH);
    delay(msgDelay);
    digitalWrite(fav2Pin, LOW);
  }
  if (twitterCommand == "volumeto10"){
    //turn volume up to full
    for (int i = 0; i < 10; i++){
      digitalWrite(volUpPin, HIGH);
      delay(msgDelay);
      digitalWrite(volUpPin, LOW);
    }
  }
  if (twitterCommand == "volumeto5"){
    for (int i = 0; i < 10; i++){
       digitalWrite(volDownPin, HIGH);
       delay(msgDelay);
       digitalWrite(volDownPin, LOW);
    }
    for (int i = 0; i < 5; i++){
      digitalWrite(volUpPin, HIGH);
      delay(msgDelay);
      digitalWrite(volUpPin, LOW);
    }
  }
  String minutes, seconds;
  if (minute() < 10) {
    minutes = "0" + String(minute());
  } else {
    minutes = String(minute());f
  }
  if (second() < 10) {
    seconds = "0" + String(second());
  } else {
    seconds = String(second());
  }
  updateTwitterStatus("Command " + twitterCommand + " received and executed at " + String(hour()) + ":" + minutes + ":" + seconds + ". ");
}

time_t getNtpTime()
{
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  String twitterCommand = checkTalkBack();
  deleteDuplicateFromTalkBackQueue();
  if (twitterCommand != "") {
    reactToTwitterControl(twitterCommand);
  }
  delay(checkTalkBackInterval);
  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
  }
}
