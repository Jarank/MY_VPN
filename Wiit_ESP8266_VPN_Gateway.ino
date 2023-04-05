#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Set up the Wi-Fi client
const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";
WiFiClient wifiClient;

// Set up the MQTT client
const char* mqttServer = "your_MQTT_broker_address";
const int mqttPort = 1883;
const char* mqttUser = "your_MQTT_username";
const char* mqttPassword = "your_MQTT_password";
PubSubClient mqttClient(wifiClient);

// Set up the UART
const int uartBaudRate = 9600;

void setup() {
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }

  // Connect to MQTT broker
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCredentials(mqttUser, mqttPassword);
  while (!mqttClient.connected()) {
    if (mqttClient.connect("WittyESP8266")) {
      mqttClient.subscribe("uart_to_mqtt");
    } else {
      delay(1000);
    }
  }

  // Set up the UART
  Serial.begin(uartBaudRate);
}

void loop() {
  // Check for incoming UART data
  if (Serial.available() > 0) {
    String uartData = Serial.readStringUntil('\n');
    mqttClient.publish("uart_to_mqtt", uartData);
  }

  // Check for incoming MQTT data
  mqttClient.loop();
}

// Callback function to handle incoming MQTT messages
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Check if the incoming MQTT message is for the UART
  if (strcmp(topic, "mqtt_to_uart") == 0) {
    String mqttData = "";
    for (int i = 0; i < length; i++) {
      mqttData += (char)payload[i];
    }
    Serial.print(mqttData);
  }
}
