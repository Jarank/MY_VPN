#include <WiFi.h>
#include <PubSubClient.h>


// Update these with values suitable for your network.
const char* ssid = "4G UFI-1656";
const char* password = "012xxxxx0";
const char* mqtt_server = "vpn.osszone.com";
const int RELAY1=16;
const int RELAY2=17; 

#define mqtt_port 1883
#define MQTT_USER "ossxxxxx"   //  user
#define MQTT_PASSWORD "ossxxxx"  // passwd
#define MQTT_SERIAL_PUBLISH_CH "/icircuit/ESP32/serialdata/tx"
#define MQTT_SERIAL_RECEIVER_CH "/icircuit/ESP32/serialdata/rx"


WiFiClient wifiClient;

PubSubClient client(wifiClient);

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
    randomSeed(micros());
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32-CTL1";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(),MQTT_USER,MQTT_PASSWORD)) {
      Serial.println("connected");
//**********      //Once connected, publish an announcement... 
      client.publish("/icircuit/presence/ESP32/", "hello world ESP32-CTL1");
      // ... and resubscribe
      client.subscribe(MQTT_SERIAL_RECEIVER_CH);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte *payload, unsigned int length) {
    Serial.println("##-------new message from broker-----##");
    Serial.print("#channel:");
    Serial.println(topic);
    Serial.print("#data:");
    Serial.write(payload, length);
   if (!strncmp((char *)payload, "D16ON", length)) {
            digitalWrite(RELAY1, HIGH);
            //powerState = true;
            client.publish(MQTT_SERIAL_PUBLISH_CH, "D16ON", true);
           // Serial.print("D16ON"); 
        } else if (!strncmp((char *)payload, "D16OFF", length)) {
            digitalWrite(RELAY1, LOW);
            //powerState = false;
            client.publish(MQTT_SERIAL_PUBLISH_CH, "D16OFF", true);
           // Serial.print("D16OFF"); 
        } else if (!strncmp((char *)payload, "D17ON", length)) {
            digitalWrite(RELAY2, HIGH);
            //powerState = false;
            client.publish(MQTT_SERIAL_PUBLISH_CH, "D17ON", true);
            //Serial.print("D17ON"); 
        } else if (!strncmp((char *)payload, "D17OFF", length)) {
            digitalWrite(RELAY2, LOW);
            //powerState = false;
            client.publish(MQTT_SERIAL_PUBLISH_CH, "D17OFF", true);
            //Serial.print("D17OFF");              
        }
        
        Serial.println();
}

void setup() {
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);

  digitalWrite(RELAY1, LOW); 
  digitalWrite(RELAY2, LOW);
  
  Serial.begin(115200);
  Serial.setTimeout(500);// Set time out for 
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  reconnect();
}

void publishSerialData(char *serialData){
  if (!client.connected()) {
    reconnect();
  }
  client.publish(MQTT_SERIAL_PUBLISH_CH, serialData);
}
void loop() {
   client.loop();
   if (Serial.available() > 0) {
     char mun[501];
     memset(mun,0, 501);
     Serial.readBytesUntil( '\n',mun,500);
     publishSerialData(mun);
   }
 }
