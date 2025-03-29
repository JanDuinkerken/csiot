#include<ESP8266WiFi.h>
#include<WiFiClientSecure.h>
#include<PubSubClient.h>

#include<ctype.h>

#include "certs.h"

const char* SSID = "";
const char* PASSWORD = "";

const char* MQTT_SERVER = "";
const int MQTT_PORT = 0;
const char* MQTT_USER = "";
const char* MQTT_PASSWD = "";
const char* MQTT_TOPIC_CONN = "munics/iot/connections";
const char* MQTT_TOPIC_READ = "munics/iot/lightCommand";
const char* MQTT_TOPIC_POST = "munics/iot/lightState";

X509List CERT(CA_CERT);

WiFiClientSecure espClient;
PubSubClient client(espClient);

void connectToWiFi() {
    WiFi.begin(SSID, PASSWORD);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected to WiFi");
}

void connectToMQTT() {
    while (!client.connected()) {
        String client_id = "ESP8266";
        Serial.printf("Connecting to MQTT Broker ...\n");
        if (client.connect(client_id.c_str(), MQTT_USER, MQTT_PASSWD)) {
            Serial.println("Connected to MQTT broker");
            client.subscribe(MQTT_TOPIC_READ);
            client.publish(MQTT_TOPIC_CONN, "ESP8266 connected");  // Publish message upon connection
        } else {
            Serial.print("Failed to connect to MQTT broker, rc=");
            Serial.print(client.state());
            Serial.println(" Retrying in 5 seconds.");
            delay(5000);
        }
    }
}

void mqttCallback(char *topic, byte *payload, unsigned int length) {
  // Payload can only have 1 char
    if (length > 1) {
      Serial.println("Message received is not valid.\n");
      return;
    }

  // Payload has to be numeric
    if (!isDigit((char) payload[0])) {
      Serial.println("Received message is not a number.\n");
      return;
    }

  // Print payload
    Serial.print("Message received on topic: ");
    Serial.println(topic);
    Serial.print("Message: ");
    Serial.println((char) payload[0]);
    
  // Led logic
    switchLed(atoi((char*)payload));
}

void switchLed(int state){
  if(state){
    // We send the color for ON state
    client.publish(MQTT_TOPIC_POST, "#00FF00");
    digitalWrite(LED_BUILTIN, !HIGH);
  } else {
    // We send the color for OFF state
    client.publish(MQTT_TOPIC_POST, "#FF0000");
    digitalWrite(LED_BUILTIN, !LOW);
  }
}

void setup() {
  // Configure baud rate for serial port
  Serial.begin(115200);
  delay(10);

  // Set LED as output an default off
  // We use !LOW because this board has the LOW and HIGH states flipped
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, !LOW);

  // Connect to WIFI
  connectToWiFi();

  // Set time
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  // Set root ca certificate
  espClient.setTrustAnchors(&CERT);

  // Connect to MQTT broker
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setKeepAlive(60);
  client.setCallback(mqttCallback);
  connectToMQTT();
}

void loop() {
  if (!client.connected()) {
    connectToMQTT();
  }
  client.loop();
}
