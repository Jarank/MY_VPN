#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

// WiFi credentials
//const char* ssid ="";
//const char* password = "22514684";
///////////////////////////////////////
char client_name[20] = "ESP8266-AP";
///////////////////////////////////////
char mqtt_topic_luminosity[50];
char mqtt_topic_led[50];
char mqtt_topic_relay[50];
char mqtt_topic_status[50];
char mqtt_topic_countdown[50];
//sprintf(mqtt_topic_luminosity, "AngSila/%s/LDR", client_name);
// MQTT broker credentials
char mqtt_server[40] = "vpn.osszone.com";
char mqtt_username[20] = "ossuser1" ;
char mqtt_password[20] = "osspwd1";
char mqtt_port[5] = "1883";
char mqtt_client_name[20] = "ESP8266-AP";
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
int countdown = 300 ;

// flags to keep track of relay and LED status
bool relayStatus = false;
String ledStatus = "off";
// create a WiFi client instance
WiFiClient espClient;
// create a MQTT client instance
PubSubClient mqttClient(espClient);

void setup() {
  // set LED and relay pins as outputs
  // ESP.wdtEnable(3000);

  //sprintf(mqtt_topic_luminosity, "AngSila/%s/LDR", mqtt_client_name);
  //sprintf(mqtt_topic_led, "AngSila/%s/LED", mqtt_client_name);
  //sprintf(mqtt_topic_relay, "AngSila/%s/RELAY", mqtt_client_name);
  //sprintf(mqtt_topic_status, "AngSila/%s/STATUS", mqtt_client_name);
  //sprintf(mqtt_topic_countdown, "AngSila/%s/COUNTDOWN", mqtt_client_name);
 
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
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);

//    Serial.println("Connecting to WiFi...");
//  }
//  Serial.println("WiFi connected");
// Wifimanager //
// unsigned long previous_time = 0;
//unsigned long delay = 20000;  // 20 seconds delay
    bool res;
// The extra parameters to be configured (can be either global or just in the setup)
// After connecting, parameter.getValue() will get you the configured value
// id/name placeholder/prompt default length
  WiFiManager wm;
  wm.setTimeout(120);
  WiFiManagerParameter custom_mqtt_server("mqtt_server", "mqtt server", mqtt_server, 40);
  wm.addParameter(&custom_mqtt_server);  
  WiFiManagerParameter custom_mqtt_port("mqtt_port", "mqtt port", mqtt_port, 5);
   wm.addParameter(&custom_mqtt_port);
  WiFiManagerParameter custom_mqtt_username("mqtt_username", "mqtt username", mqtt_username, 20);
   wm.addParameter(&custom_mqtt_username);
  WiFiManagerParameter custom_mqtt_password("mqtt_password", "mqtt password", mqtt_password, 20);
   wm.addParameter(&custom_mqtt_password);
  WiFiManagerParameter custom_mqtt_client_name("client_name", "mqtt client name", mqtt_client_name, 20);
   wm.addParameter(&custom_mqtt_client_name);
  //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
  


    //res = wm.autoConnect(); // auto generated AP name from chipid
    // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
    res = wm.autoConnect(mqtt_client_name,"ossadmin"); // password protected ap

    if(!res) {
        Serial.println("Failed to connect");
           ESP.reset();
           delay(5000);
        // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
        //ESP.wdtDisable();
         strcpy(mqtt_server, custom_mqtt_server.getValue());
         strcpy(mqtt_port, custom_mqtt_port.getValue());
         strcpy(mqtt_username, custom_mqtt_username.getValue());
         strcpy(mqtt_password, custom_mqtt_password.getValue());
         strcpy(mqtt_client_name, custom_mqtt_client_name.getValue());

         sprintf(mqtt_topic_luminosity, "AngSila/%s/LDR", mqtt_client_name);
         sprintf(mqtt_topic_led, "AngSila/%s/LED", mqtt_client_name);
         sprintf(mqtt_topic_relay, "AngSila/%s/RELAY", mqtt_client_name);
         sprintf(mqtt_topic_status, "AngSila/%s/STATUS", mqtt_client_name);
         sprintf(mqtt_topic_countdown, "AngSila/%s/COUNTDOWN", mqtt_client_name);

    }
  }
 
  // connect to MQTT broker
  mqttClient.setServer(mqtt_server, atoi(mqtt_port));
  mqttClient.setCallback(callback);

  while (!mqttClient.connected()) {
    Serial.println("Connecting to MQTT broker...");
    if (mqttClient.connect(mqtt_client_name, mqtt_username, mqtt_password)) {
      Serial.println("MQTT connected");
      mqttClient.subscribe(mqtt_topic_led);
      mqttClient.subscribe(mqtt_topic_relay);
    } else {
      Serial.print("MQTT failed, rc=");
      Serial.println(mqttClient.state());
      delay(3000);
      ESP.restart();
    }
  }

  // blink status LED to indicate MQTT connection status
  // blinkStatusLED(mqttClient.connected());
}

void loop() {
   // check if the WiFi connection is still active
  reconnect() ;
  delay(10);
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
      wifiManager.startConfigPortal(mqtt_client_name);
      digitalWrite(BUILTIN_LED_PIN, HIGH);
    }
  }

 
  mqttClient.publish(mqtt_topic_led, ledStatus.c_str());
  mqttClient.publish(mqtt_topic_countdown, String(countdown).c_str());
  mqttClient.publish(mqtt_topic_relay, String(relayStatus).c_str());
  
  countdown = countdown -1 ;
   if (countdown <= 0) {
      mqttClient.publish(mqtt_topic_countdown, String(countdown).c_str());
      delay(100);
      countdown=300 ;
      //Restart if commectiom lose.
      ESP.restart();
  }

 // reset the watchdog timer to avoid a reset
 // ESP.wdtFeed();
 // delay for a short time to avoid overwhelming the watchdog timer
 //  delay(100);
  delay(200);
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
 // blink builtin led to indicate the device is running
  digitalWrite(BUILTIN_LED_PIN, LOW);
  delay(100);
  digitalWrite(BUILTIN_LED_PIN, HIGH);
  delay(100);    
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

// delay(2000); // loop delay

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
    delay(2000);
    digitalWrite(STATUS_PIN, LOW);
    delay(1000);
  }
}

void reconnect() {

 //loop while we wait for connection
   bool res;
  //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wm;
    res = wm.autoConnect(); // auto generated AP name from chipid
    // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
    //res = wm.autoConnect(client_name,"ossadmin"); // password protected ap

    if(!res) {
        Serial.println("Failed to connect");
        delay(2000);
        ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
        //ESP.wdtDisable();
    }
 

}

