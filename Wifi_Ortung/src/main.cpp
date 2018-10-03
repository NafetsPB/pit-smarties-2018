#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>


#define maxCh 13 // max Channel -> US = 11, EU = 13, Japan = 14

#define whiteListSize 2
//Mac Adressen (Smartphone-A, Smartphone-B)
uint8_t macWhiteList[][6] = {{0xAA,0xBB,0xCC,0xDD,0xEE,0xFF},{0xFF,0xEE,0xDD,0xCC,0xBB,0xAA}};

uint8_t messungen[whiteListSize];
int RSSISumme[whiteListSize];

#define blackListSize 1
uint8_t macBlackList[][6] = {{255,255,255,255,255,255}};

#define maxMacCount 500

int seenMacsSize = 0;
uint8_t macSeen[maxMacCount][6];

bool useWhiteList = false;
bool useBlackList = true;

unsigned long lastTime = 0;
unsigned long timeDelay = 10000;  //10s

char mac_ToHex[18];   //used in maxToString() has to be global to work

//===== Run-Time variables Wifi=====//
unsigned long prevTime   = 0;
unsigned long curTime    = 0;
unsigned long pkts       = 0;
unsigned long no_deauths = 0;
unsigned long deauths    = 0;
int curChannel           = 1;
unsigned long maxVal     = 0;
double multiplicator     = 0.0;
bool canBtnPress = true;

//MQQT Stuff
// Update these with values suitable for your network.
const char* ssid = "SSID";
const char* password = "PASSWD";
const char* mqtt_server = "172.16.0.6";
const char* mqttChannel = "home/WifiRSSIData";


WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;

int value = 0;

//void reconnect();
char* macToString(uint8_t *buf);


/*void setup_wifi() {

  wifi_promiscuous_enable(0);
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
*/

void sendRSSIsToServer()
{
  for(int i = 0; i< whiteListSize; i++)
  {
    digitalWrite(LED_BUILTIN, LOW); 
    char msg[22];
    memset(msg,22,0);
    strncpy(msg,macToString(macWhiteList[i]),18);

    if(messungen[i] != 0)
    {
      int avg = (int) RSSISumme[i]/messungen[i];
    //Clean//Serial.print(avg);

      strncpy(msg+18,String(avg).c_str(),5);
    }
    else
    {
      strncpy(msg+18,"0",5);
      //Serial.print("0");
    }
    
    //client.publish(mqttChannel, msg);
    Serial.print("MQTT.");
    Serial.print(mqttChannel);
    Serial.print(".");
    Serial.print(msg);
    Serial.println(".");

    RSSISumme[i] = 0;
    messungen[i] = 0;
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH); 
  }
}

void sendMacsToServer()
{
  for(int i = 0; i< seenMacsSize; i++)
  {
    digitalWrite(LED_BUILTIN, LOW); 
    char msg[22];
    memset(msg,22,0);
    strncpy(msg,macToString(macSeen[i]),18);
    strncpy(msg+18,String(seenMacsSize).c_str(),5);
    //client.publish(mqttChannel, msg);
    Serial.print("MQTT.");
    Serial.print("home/WifiMacData");
    Serial.print(".");
    Serial.print(msg);
    Serial.println(".");

    delay(10);
    digitalWrite(LED_BUILTIN, HIGH); 
  }
  Serial.print("MQTT.");
  Serial.print("home/WifiMacCount");
  Serial.print(".");
  Serial.print(seenMacsSize);
  Serial.println(".");
  
  seenMacsSize = 0;
}

/*
//is the mac addres in the Blacklist
bool inBlacklist(uint8_t *pMac)
{
  bool hit = false;
  for(int i = 0; i< blackListSize && hit == false; i++)
  {
    hit = true;
    for(int j = 0; j<6 && !hit ;j++)
    {
      if(pMac[j] != macBlackList[i][j])
      {
        hit = false;
      }
    }
  }
  return hit;
}

//is the mac addres in the Whitelist
bool inWhitelist(uint8_t *pMac)
{
  bool hit = false;
  for(int i = 0; i< whiteListSize && hit == false; i++)
  {
    hit = true;
    for(int j = 0; j<6 && !hit;j++)
    {
      if(pMac[j] != macWhiteList[i][j])
      {
        hit = false;
      }
    }
  }
  return hit;
}*/

bool macInList(uint8_t *pMac, uint8_t pList[][6], int pListSize)
{
  bool hit = false;
  for(int i = 0; i< pListSize && hit == false; i++)
  {
    hit = true;
    for(int j = 0; j<6 && hit ;j++)
    {
      if(pMac[j] != pList[i][j])
      {
        hit = false;
      }
    }
  }
  return hit;
}

int getMacNr(uint8_t *pMac)
{
  bool hit = false;
  for(int i = 0; i< whiteListSize && hit == false; i++)
  {
    hit = true;
    for(int j = 0; j<6;j++)
    {
      if(pMac[j] != macWhiteList[i][j])
      {
        hit = false;
      }
    }
    if(hit == true)
    { 
      return i;
    }
  }
  return -1;
}

char* macToString(uint8_t *buf)
{
  char hex[17]="0123456789ABCDEF";
  for(int i = 0; i<6;i++)
  {
    mac_ToHex[i*3] = hex[(buf[i] >>4) & 0x0F];
    mac_ToHex[i*3+1] = hex[buf[i] & 0x0F];
    mac_ToHex[i*3+2] = ':';
  }
  mac_ToHex[17] = ' ';
  return mac_ToHex;
}

void printMac(uint8_t *buf)
{
  for(int i = 0; i<5;i++)
  {
    Serial.print(buf[i],HEX);
    Serial.print(':');
  }
  Serial.print(buf[5],HEX);
}

void sniffer(uint8_t *buf, uint16_t len) {    
    uint8_t mac[6];
    memcpy(mac,buf+16,6);
    
    if(len==128)
    {
      
      if(((!macInList(mac,macBlackList,blackListSize) == useBlackList) || !useBlackList) && ((macInList(mac,macWhiteList,whiteListSize) == useWhiteList) || !useWhiteList))
      {
        
        //Serial.println();
        if(!macInList(mac,macSeen,seenMacsSize+1))
        {
          printMac(mac);
          Serial.println("added");
          memcpy(macSeen[seenMacsSize],mac,6);
          
          if(seenMacsSize < maxMacCount)
          {
            seenMacsSize++;
          }
        }

        int macNr = getMacNr(mac);
        if(macNr >=0 && macNr < whiteListSize)
        {
          //Serial.println("packet");
          messungen[macNr]= messungen[macNr] + 1;
          RSSISumme[macNr] = RSSISumme[macNr] + (int8_t) buf[0];
          
        }
      }
      
    }
      
}

void startSniffing()
{
    /* setup wifi */
    wifi_set_opmode(STATION_MODE);
    wifi_promiscuous_enable(0);
    WiFi.disconnect();
    wifi_set_promiscuous_rx_cb(sniffer);
    wifi_set_channel(curChannel);
    wifi_promiscuous_enable(1);
}


void setup() {
    // put your setup code here, to run once:
    Serial.begin(9600);
    delay(100);
    
    //Serial.println("hallo");
    pinMode(LED_BUILTIN, OUTPUT);
    startSniffing();
}

void loop() {
    curChannel++;    
    if (curChannel > maxCh) 
        curChannel = 1;

    if(lastTime + timeDelay < millis())
    {
      //Serial.print("printStuff:");
      sendRSSIsToServer();
      sendMacsToServer();
      //delay(5000);
      //startSniffing();
      
      lastTime = millis();
    }
    //Serial.println(WiFi.RSSI(0));

}

