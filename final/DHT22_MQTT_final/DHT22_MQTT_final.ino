//Library(ZIP)：async_mqtt_client、AsyncTCP、pubsubclient
//Library：DHT sensor library、Adafruit Unified Sensor、ESP32 AnalogWrite
//DHT22：3.3V 4  GND
//L9110：5V GND 16(RX2) 17(TX2)
//LED：GND   1(TX/TX0)

#include "DHT.h"
#include <WiFi.h>
#include <AsyncMqttClient.h>
#include <analogWrite.h>
#include <PubSubClient.h>
#include <Wire.h>

// Replace the next variables with your SSID/Password combination
const char* ssid = "WY1010";
const char* password = "20041010311";

// Add your MQTT Broker IP address, example:
//const char* mqtt_server = "192.168.162.158";
const char* mqtt_server = "192.168.112.158";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

#define DHTPIN 4
#define DHTTYPE DHT22
#define IA 16
#define IB 17
#define LED 1

// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);

float temperature = 0;
float humidity = 0;

void setup() {
  pinMode(IA, OUTPUT);
  pinMode(IB, OUTPUT);
  pinMode(LED, OUTPUT);
  Serial.begin(115200);
  dht.begin();
  // default settings
  // (you can also pass in a Wire library object like &Wire2)
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  analogWrite(LED, 0);
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

void callback(char* topic, byte* message, unsigned int length) {
  /*Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");*/
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    //Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  //Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == "esp32/output") {
    //Serial.print("風扇");
    if(messageTemp == "on"){
      //Serial.println("開");
      analogWrite(IA, 90);
      analogWrite(IB, 0);
    }
    else if(messageTemp == "off"){
      //Serial.println("關");
      analogWrite(IA, 0);
      analogWrite(IB, 0);
    }
  }
  if (String(topic) == "esp32/light_output") {
    //Serial.print("電燈");
    if(messageTemp == "on"){
      //Serial.println("開");
      analogWrite(LED, 150);
    }
    else if(messageTemp == "off"){
      //Serial.println("關");
      analogWrite(LED, 0);
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/output");
      client.subscribe("esp32/light_output");
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
  if (now - lastMsg > 5000) {
    lastMsg = now;
    
    // Temperature in Celsius
    temperature = dht.readTemperature();
    // Uncomment the next line to set temperature in Fahrenheit 
    // (and comment the previous temperature line)
    //temperature = 1.8 * bme.readTemperature() + 32; // Temperature in Fahrenheit
    
    // Convert the value to a char array
    char tempString[8];
    dtostrf(temperature, 1, 2, tempString);
    //Serial.print("Temperature: ");
    //Serial.println(tempString);
    client.publish("esp32/temperature", tempString);

    humidity = dht.readHumidity();
    
    // Convert the value to a char array
    char humString[8];
    dtostrf(humidity, 1, 2, humString);
    //Serial.print("Humidity: ");
    //Serial.println(humString);
    client.publish("esp32/humidity", humString);
  }
}
