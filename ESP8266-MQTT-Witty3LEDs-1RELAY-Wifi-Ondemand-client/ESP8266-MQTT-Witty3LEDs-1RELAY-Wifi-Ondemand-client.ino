#include <mqttif.h>
#include <tweetnacl.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

// WiFi credentials
//const char* ssid ="";
//const char* password = "22514684";


///////////////////////////////////////
const char* client_name = "ESP8266-C5";
///////////////////////////////////////
char mqtt_topic_luminosity[50];
char mqtt_topic_led[50];
char mqtt_topic_relay[50];
char mqtt_topic_status[50];
//sprintf(mqtt_topic_luminosity, "AngSila/%s/LDR", client_name);
// MQTT broker credentials
const char* mqtt_server = "vpn.osszone.com";
const char* mqtt_username = "ossuser1";
const char* mqtt_password = "osspwd1";
//const char* mqtt_topic_luminosity = "AngSila/ESP-86-100/LDR";
//const char* mqtt_topic_led = "AngSila/ESP-86-100/RGB";
//const char* mqtt_topic_relay = "AngSila/ESP-86-100/RELAY";
//const char* mqtt_topic_status = "AngSila/ESP-86-100/STATUS";
             
// Witty ESP8266 pins
const int LED_RED_PIN = 15;
const int LED_GREEN_PIN = 12;
const int LED_BLUE_PIN = 13;
const int LDR_PIN = A0;
const int RELAY_PIN = 14; // changed to GPIO0
const int STATUS_PIN = 2;
#define PUSHBUTTON_PIN 4
#define BUILTIN_LED_PIN 2

// flags to keep track of relay and LED status
bool relayStatus = false;
String ledStatus = "off";

// create a WiFi client instance
WiFiClient espClient;

// create a MQTT client instance
PubSubClient mqttClient(espClient);

void setup() {
  // set LED and relay pins as outputs
  
  sprintf(mqtt_topic_luminosity, "AngSila/%s/LDR", client_name);
  sprintf(mqtt_topic_led, "AngSila/%s/LED", client_name);
  sprintf(mqtt_topic_relay, "AngSila/%s/RELAY", client_name);
  sprintf(mqtt_topic_status, "AngSila/%s/STATUS", client_name);
 
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(STATUS_PIN, OUTPUT);

  Serial.begin(115200);
  
 // connect to WiFi
 // WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
//    Serial.println("Connecting to WiFi...");
//  }
//  Serial.println("WiFi connected");
// Wifimanager //
    bool res;
  //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
   WiFiManager wm;
    res = wm.autoConnect(); // auto generated AP name from chipid
    // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
    
    //res = wm.autoConnect(client_name,"ossadmin"); // password protected ap

    if(!res) {
        Serial.println("Failed to connect");
        // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
    }
  }
 

  // connect to MQTT broker
  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(callback);
  while (!mqttClient.connected()) {
    Serial.println("Connecting to MQTT broker...");
    if (mqttClient.connect(client_name, mqtt_username, mqtt_password)) {
      Serial.println("MQTT connected");
      mqttClient.subscribe(mqtt_topic_led);
      mqttClient.subscribe(mqtt_topic_relay);
    } else {
      Serial.print("MQTT failed, rc=");
      Serial.println(mqttClient.state());
      delay(1000);
    }
  }

  // blink status LED to indicate MQTT connection status
  blinkStatusLED(mqttClient.connected());
}

void loop() {
  mqttClient.loop(); // keep the MQTT connection alive

  // read the LDR value and adjust the LED colors accordingly
  int ldrValue = analogRead(LDR_PIN);
  Serial.print("Luminosity: ");
  Serial.println(ldrValue);
  mqttClient.publish(mqtt_topic_luminosity, String(ldrValue).c_str());
 
  if (ldrValue < 256) {
    digitalWrite(LED_RED_PIN, HIGH);
    digitalWrite(LED_GREEN_PIN, LOW);
    digitalWrite(LED_BLUE_PIN, LOW);
    ledStatus = "red";
  } else if (ldrValue >= 256 && ldrValue < 512) {
    digitalWrite(LED_RED_PIN, LOW);
    digitalWrite(LED_GREEN_PIN, HIGH);
    digitalWrite(LED_BLUE_PIN, LOW);
    ledStatus = "green";
  } else {
    digitalWrite(LED_RED_PIN, LOW);
    digitalWrite(LED_GREEN_PIN, LOW);
    digitalWrite(LED_BLUE_PIN, HIGH);
    ledStatus = "blue";
  }

 int counter = 0;
  while (digitalRead(PUSHBUTTON_PIN) == LOW) {
    delay(1000);
    counter++;
    if (counter >= 5) {
      WiFiManager wifiManager;
      wifiManager.setConfigPortalTimeout(120);
      wifiManager.setConnectTimeout(10);
      wifiManager.setClass("on demand");
      wifiManager.startConfigPortal(client_name);
    }
  }

  // blink builtin led to indicate the device is running
  digitalWrite(BUILTIN_LED_PIN, HIGH);
  delay(500);
  digitalWrite(BUILTIN_LED_PIN, LOW);
  delay(500);
  
 
mqttClient.publish(mqtt_topic_led, ledStatus.c_str());

}
// callback function for MQTT messages
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received on topic: ");
  Serial.println(topic);

  // handle LED messages
  if (strcmp(topic, mqtt_topic_led) == 0) {
    if (payload[0] == 'o' && payload[1] == 'n') {
      digitalWrite(LED_RED_PIN, HIGH);
      digitalWrite(LED_GREEN_PIN, HIGH);
      digitalWrite(LED_BLUE_PIN, HIGH);
      ledStatus = "on";
    } else if (payload[0] == 'o' && payload[1] == 'f' && payload[2] == 'f') {
      digitalWrite(LED_RED_PIN, LOW);
      digitalWrite(LED_GREEN_PIN, LOW);
      digitalWrite(LED_BLUE_PIN, LOW);
      ledStatus = "off";
    }
    mqttClient.publish(mqtt_topic_led, ledStatus.c_str());
    
  }

  // handle relay messages
  if (strcmp(topic, mqtt_topic_relay) == 0) {
    if (payload[0] == 'O' && payload[1] == 'F' && payload[2] == 'F' ) {
      relayStatus = false;
      digitalWrite(RELAY_PIN, LOW);
      mqttClient.publish(mqtt_topic_relay, "OFF");
    } else if (payload[0] == 'O' && payload[1] == 'N' ) {
      relayStatus = true;
      digitalWrite(RELAY_PIN, HIGH);
      mqttClient.publish(mqtt_topic_relay, "ON");
    }
  }
 
  }
// }

// function to blink the status LED to indicate MQTT connection status
void blinkStatusLED(bool connected) {
  if (connected) {
    digitalWrite(STATUS_PIN, HIGH);
    delay(500);
    digitalWrite(STATUS_PIN, LOW);
    delay(500);
  } else {
    digitalWrite(STATUS_PIN, HIGH);
    delay(100);
    digitalWrite(STATUS_PIN, LOW);
    delay(900);
  }
}
