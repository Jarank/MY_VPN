#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <ESP8266OpenVPN.h>

ESP8266WiFiMulti WiFiMulti;
ESP8266OpenVPN openvpn;

WiFiServer server(23);

void setup() {
  Serial.begin(115200);

  // connect to WiFi
  WiFiMulti.addAP("SSID", "password");
  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

  // connect to OpenVPN server
  openvpn.connect("server_address", 1194, "username", "password");

  // start Telnet server
  server.begin();
}

void loop() {
  // wait for new Telnet connection
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New Telnet connection");
    
    // create new OpenVPN connection
    ESP8266OpenVPN vpn;
    vpn.connect("server_address", 1194, "username", "password");

    // create TCP/IP connection to remote server through VPN tunnel
    WiFiClient remote_client;
    while (!remote_client.connected()) {
      if (vpn.available()) {
        char c = vpn.read();
        remote_client.write(c);
      }
      if (client.available()) {
        char c = client.read();
        vpn.write(c);
      }
    }

    // forward data between Telnet client and remote server
    while (client.connected() && remote_client.connected()) {
      if (client.available()) {
        char c = client.read();
        remote_client.write(c);
      }
      if (remote_client.available()) {
        char c = remote_client.read();
        client.write(c);
      }
    }

    // close OpenVPN connection and release resources
    remote_client.stop();
    vpn.disconnect();
    Serial.println("Telnet connection closed");
  }

  // check for OpenVPN data
  if (openvpn.available()) {
    char c = openvpn.read();
    Serial.write(c);
  }
}
