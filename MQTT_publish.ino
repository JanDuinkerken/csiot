#include<ESP8266WiFi.h>
#include<PubSubClient.h>

const char* SSID = "";
const char* PASSWORD = "";

const char* MQTT_SERVER = "";
const int MQTT_PORT = 0;
const char* MQTT_USER = "";
const char* MQTT_PASSWD = "";

WiFiClient ESP_CLIENT;
PubSubClient client(ESP_CLIENT);

int DATA = 0;
char PUBLISH_DATA[50];

void setup() {
  // Configure baud rate for serial port
  Serial.begin(115200);
  delay(10);

  // Connect to WIFI
  Serial.println(SSID);
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("Connected");

  Serial.println(WiFi.localIP());

  // Connect to MQTT broker
  client.setServer(MQTT_SERVER, MQTT_PORT);

  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");

    if (client.connect("ESP8266", MQTT_USER, MQTT_PASSWD)) {
      Serial.println("Connected");
    } else {
      Serial.println("Failed");
      delay(2000);
    }
  }

  // Publish first hello world message
  client.publish("test/munics", "Hello, world!");
}

void loop() {
  // Get data from sensors
  String SensorData = "Data: ";
  SensorData.concat(DATA);
  SensorData.toCharArray(PUBLISH_DATA, SensorData.length() + 1);

  // Publish data to topic
  Serial.println(SensorData);
  client.publish("test/munics", PUBLISH_DATA);
  client.loop();
  DATA++;
}
