#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>

#define MQTT_PUBLISH_TOPIC "home"
#define MQTT_SUBSCRIBE_DISPLAY "home/Display/#"
#define display_base "home/Display/"
#define mqtt_d_fenster "home/Display/fenster"
#define mqtt_d_temp "home/Display/temp"
#define mqtt_d_feucht "home/Display/feucht"
#define mqtt_d_light1 "home/Display/licht1"
#define mqtt_d_light2 "home/Display/licht2"
#define mqtt_d_bett "home/Display/bett"
#define MQTT_SUBSCRIBE_RFID "home/rfid_access"
#define ssid "SSID"
#define password "PASSWD"
#define mqtt_server "172.16.0.6"
#define mqtt_port 1883
#define clientId "NodeMCU-Display"

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

WiFiClient wifiClient;
PubSubClient client(wifiClient);

String fenster = "Fenster: zu";
String temperatur = "Temperatur: 0 *C";
String feuchtigkeit = "Feuchtigkeit: 0%";
String licht1 = "Grosses Licht: aus";
String licht2 = "Schlaflicht: an";
String bett = "Bett: leer";

void setup() {
  Serial.begin(115200);
  tft.init();
  tft.setRotation(1);
  
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  tft.setCursor(0,0,2);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.println("Home-Display:");
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
  auswertung(topic, message);
}

void auswertung(char* topic, String message){
  tft.setCursor(0,0,2);
  tft.fillScreen(TFT_BLACK);
  String t = String(topic);
  if(t == MQTT_SUBSCRIBE_DISPLAY){
    temperatur = message;
    String willkommen = "** Homestatus **";
    printMessage(willkommen);
  }
  if(t == MQTT_SUBSCRIBE_RFID){
    if(message == "Error"){
      printError();
    } else {
      String willkommen = "Willkommen " + message + "!";
      printMessage(willkommen);
    }
  } else {
    auswertung(t, message);
  }
}

void auswertung(String topic, String message){
  if(topic == mqtt_d_temp){
    temperatur = "Temperatur: " + message;
  } else if (topic == mqtt_d_feucht){
    feuchtigkeit = "Feuchtigkeit:" + message;
  } else if(topic == mqtt_d_fenster){
    fenster = "Fenster: " + message;
  } else if(topic == mqtt_d_light1){
    licht1 = "Grosses Licht: " + message;
  } else if(topic == mqtt_d_light2){
    licht2 = "Schlaflampe: " + message;
  } else if(topic == mqtt_d_bett){
    bett = "Bett: " + message;
  }
}

void printMessage(String willkommen){
  tft.setTextColor(TFT_GREEN);
  tft.println(willkommen);
  tft.setTextColor(TFT_WHITE);
  tft.println(temperatur);
  tft.println(feuchtigkeit);
  tft.println(fenster);
  tft.println(licht1);
  tft.println(licht2);
  tft.println(bett);
  delay(10000);
  tft.fillScreen(TFT_BLACK);
}

void printError(){
  tft.setTextColor(TFT_RED);
  tft.println(" / / / / / / / / ");
  tft.println("  Access denied  ");
  tft.println(" / / / / / / / / ");
  delay(5000);
  tft.fillScreen(TFT_BLACK);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientId)) {
      Serial.println("connected");
      client.publish(MQTT_PUBLISH_TOPIC, "hello world");
      client.subscribe(MQTT_SUBSCRIBE_DISPLAY);
      client.subscribe(MQTT_SUBSCRIBE_RFID);
      Serial.println("Hat subscribed.");
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
}
