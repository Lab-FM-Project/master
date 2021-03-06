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
//#include <pins_nodeMCU.h>
//#include <MQTT.h>
#define volUpPin D1
/*#define volDownPin
#define freqUpPin
#define freqDownPin
*/
#define msgDelay 100

/*#define user twdgskxg
#define password vkMS2TFzBxLc
#define sslport 28296 */



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
  pinMode(volUpPin, OUTPUT);
  digitalWrite(volUpPin, LOW);
  digitalWrite(BUILTIN_LED, HIGH); //off
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
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  if (*topic == "volume" && (char)payload[0] == 'u'){
    digitalWrite(volUpPin, HIGH);
    digitalWrite(BUILTIN_LED, LOW);
    delay(msgDelay);
    digitalWrite(volUpPin, LOW);
    digitalWrite(BUILTIN_LED, HIGH);
  }
  if (*topic == "volume" && (char)payload[0] == 'd'){
    digitalWrite(volDownPin, HIGH);
    digitalWrite(BUILTIN_LED, LOW);
    delay(msgDelay);
    digitalWrite(volDownPin, LOW);
    digitalWrite(BUILTIN_LED, HIGH);
  }
  if (*topic == "frequency" && (char)payload[0] == 'u'){
    digitalWrite(freqUpPin, HIGH);
    digitalWrite(BUILTIN_LED, LOW);
    delay(msgDelay);
    digitalWrite(volUpPin, LOW);
    digitalWrite(BUILTIN_LED, HIGH);
  }
  if (*topic == "frequency" && (char)payload[0] == 'd'){
    digitalWrite(freqDownPin, HIGH);
    digitalWrite(BUILTIN_LED, LOW);
    delay(msgDelay);
    digitalWrite(freqDownPin, LOW);
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
      client.subscribe("remote_ctrl");
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

