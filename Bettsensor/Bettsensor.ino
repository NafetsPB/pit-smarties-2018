#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>

#define MQTT_PUBLISH_TOPIC "home/bett"
#define MQTT_SUBSCRIBE_LIGHT "home/light2"
#define ssid "SSID"
#define password "PASSWD"
#define mqtt_server "172.16.0.6"
#define mqtt_port 1883
#define clientId "NodeMCU-Bett"
#define PIN 12

Adafruit_NeoPixel strip = Adafruit_NeoPixel(8, PIN, NEO_GRB + NEO_KHZ800);

WiFiClient wifiClient;
PubSubClient client(wifiClient);

int sensorInput = 5; // D1
int value = 0;

void setup() {
  Serial.begin(9600);
  pinMode(sensorInput, INPUT);
  strip.begin();
  strip.setBrightness(90);
  strip.show();
  
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void setup_wifi() {
  delay(10);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientId)) {
      Serial.println("connected");
      client.publish(MQTT_PUBLISH_TOPIC, "Hello world! It is NodeMCU-Bett.");
      client.subscribe(MQTT_SUBSCRIBE_LIGHT);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.print(length);
  Serial.println(":");
  String message = "";
  for (int i = 0; i < length; i++) {
    message.concat(String((char)payload[i]));
  }
  Serial.println(message);
  if(message == "1"){
    showRainbow();
  } else {
    hideRainbow();
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  int newValue = digitalRead(sensorInput);
  Serial.println(value);
  if(value != newValue){
    String wert = String(newValue);
    char wertC[2];
    wert.toCharArray(wertC, 2);
    client.publish(MQTT_PUBLISH_TOPIC, "1");
    Serial.println("Published");
    value = newValue;
  }
  delay(500);
}

void showRainbow(){
  strip.setPixelColor(0, 204, 0, 153);
  strip.setPixelColor(1, 255, 0, 0);
  strip.setPixelColor(2, 255, 102, 0);
  strip.setPixelColor(3, 255, 204, 0);
  strip.setPixelColor(4, 102, 153, 0);
  strip.setPixelColor(5, 0, 204, 102);
  strip.setPixelColor(6, 0, 51, 204);
  strip.setPixelColor(7, 153, 0, 255);
  
  strip.show();
}

void hideRainbow(){
  for(int i = 0; i < 8; i++){
    strip.setPixelColor(i,0,0,0);
  }
  strip.show();
}
