#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>


//MQQT Stuff
// Update these with values suitable for your network.
const char* ssid = "SSID";
const char* password = "PASSWD";
const char* mqtt_server = "172.16.0.6";

WiFiClient espClient;
PubSubClient client(espClient);

void reconnect();


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

    client.setServer(mqtt_server, 1883);

    reconnect();
  }

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client_WIFI")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish(mqttChannel, "hello world");
      // ... and resubscribe

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void setup() {
    // put your setup code here, to run once:
    Serial.begin(9600);    
    Serial.println("hallo");

    setup_wifi();
}

void loop() {
    
    if(Serial.available())
    {
        char startBytes[6];
        Serial.readStringUntil('M');
        Serial.readBytes(startBytes,4);
        Serial.print("startBytes:");
        Serial.print(startBytes);
        
        if(strncmp(startBytes,"QTT.",4) == 0)
        {
            //char channel[];
            //char msg[];
            String channel = Serial.readStringUntil('.');
            String msg = Serial.readStringUntil('.');
            Serial.print("channel: ");
            Serial.println(channel);
            Serial.print("msg: ");
            Serial.println(msg);
            client.publish(channel.c_str(),msg.c_str());
        }
        else
        {
          Serial.print("not starting with \"MQTT:\"");
          Serial.read();
        }
    }

if (!client.connected()) {
        reconnect();
    }

}


