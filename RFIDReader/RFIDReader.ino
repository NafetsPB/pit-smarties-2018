#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define MQTT_PUBLISH_TOPIC "home/rfid/"
#define CLIENT_ID "NodeMCU-RFID"
#define ssid "SSID"
#define password "PASSWD"
#define mqtt_server "172.16.0.6"
#define mqtt_port 1883


constexpr uint8_t RST_PIN = 5;          // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = 4;         // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
WiFiClient wifiClient;
PubSubClient client(wifiClient);

String lastUUid = "";

void setup() {
	Serial.begin(9600);		// Initialize serial communications with the PC
	SPI.begin();			// Init SPI bus
  Serial.println("Starting infos - - - - - - - - - - - -");
	mfrc522.PCD_Init();		// Init MFRC522
	mfrc522.PCD_DumpVersionToSerial();	// Show details of PCD - MFRC522 Card Reader details
	// Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
  
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
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
    if (client.connect(CLIENT_ID)) {
      Serial.println("connected");
      client.publish(MQTT_PUBLISH_TOPIC, "Hello world");
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
	// Look for new cards
	if ( ! mfrc522.PICC_IsNewCardPresent()) {
		return;
	}

	// Select one of the cards
	if ( ! mfrc522.PICC_ReadCardSerial()) {
		return;
	} else {
    Serial.println("Karte auslesen.");
	}

  Serial.print(" UID tag :");
  byte letter;
  String uuid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     uuid.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     uuid.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  //if(uuid != lastUUid){
    char uuidC[13];
    uuid.toCharArray(uuidC, 13);
    Serial.print("UUID:");
    Serial.println(uuidC);
    client.publish(MQTT_PUBLISH_TOPIC, uuidC);
    //lastUUid = uuid;
  //}
  delay(500);
}
