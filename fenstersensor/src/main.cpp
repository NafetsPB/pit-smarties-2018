//für Fenster
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <string>
#include <sstream>

//für Neopixel
#include <Adafruit_NeoPixel.h>
 
//für Temperatur
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <stdio.h>
#include <math.h>

//für MQTT
#define ssid "SSID"
#define password "PASSWD"
#define mqtt_server "172.16.0.6"
#define mqtt_port 1883
#define clientId "NodeMCU-Fenster"

//Wlan
WiFiClient espClient;
PubSubClient client(espClient);

// Declaration and initialization of the input pin
int Fenster_Eingang = 4;
int Temperatur_Eingang = 5;
float aktuelleTemperatur = 200.0;
float aktuelleLuftfechtigkeit = 200.0;
bool FensterOffen = true;
char Messwert[6];

const int numPixels = 8;
const int pixelPinTemperatur = 2;
const int pixelPinFeuchtigkeit = 10;
const int pixelWarnleuchte = 13;

Adafruit_NeoPixel stick_Temperatur = Adafruit_NeoPixel(numPixels, pixelPinTemperatur);
Adafruit_NeoPixel stick_Feuchtigkeit = Adafruit_NeoPixel(numPixels, pixelPinFeuchtigkeit);

uint32_t blau = stick_Temperatur.Color(0x00, 0x00, 0xFF); // r, g, b - blue
uint32_t gelb = stick_Temperatur.Color(212, 180, 26);     // r, g, b - gelb
uint32_t orange = stick_Temperatur.Color(220, 70, 5);   // r, g, b - orange
uint32_t rot = stick_Temperatur.Color(255, 0, 0);         // r, g, b - rot

//Verzeichnis
std::string MQTTNachrichtFenster = "home/Wohnzimmer/Fenster1";
std::string MQTTNachrichtTemperatur = "home/Wohnzimmer/Temperatur";
std::string MQTTNachrichtFeuchtigkeit = "home/Wohnzimmer/Feuchtigkeit";

//delay für die Messungen vom DHT11
uint32_t delayMS;

DHT_Unified dht(Temperatur_Eingang, DHT11);

void setup()
{   
    stick_Temperatur.begin(); // start the NeoPixel display
    stick_Feuchtigkeit.begin(); // start the NeoPixel display

    pinMode(Fenster_Eingang, INPUT);
    pinMode(pixelWarnleuchte, OUTPUT);
    digitalWrite(pixelWarnleuchte, HIGH);

    Serial.begin(9600); // Serial output with 9600 bps
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(100);
    }
    Serial.println("Verbindung ok");
    client.setServer(mqtt_server, mqtt_port);

    dht.begin();

    sensor_t Temperatur_sensor;
    dht.temperature().getSensor(&Temperatur_sensor);
    //print Temperatur details
    Serial.println("Temperature");
    Serial.print  ("Sensor:       "); Serial.println(Temperatur_sensor.name);
    Serial.print  ("Driver Ver:   "); Serial.println(Temperatur_sensor.version);
    Serial.print  ("Unique ID:    "); Serial.println(Temperatur_sensor.sensor_id);
    Serial.print  ("Max Value:    "); Serial.print(Temperatur_sensor.max_value); Serial.println(" *C");
    Serial.print  ("Min Value:    "); Serial.print(Temperatur_sensor.min_value); Serial.println(" *C");
    Serial.print  ("Resolution:   "); Serial.print(Temperatur_sensor.resolution); Serial.println(" *C");  
    Serial.println("------------------------------------");

    sensor_t Feuchtigkeit_sensor;
    dht.humidity().getSensor(&Feuchtigkeit_sensor);
    // Print humidity Feuchtigkeit_sensor details.
    Serial.println("------------------------------------");
    Serial.println("Humidity");
    Serial.print  ("Sensor:       "); Serial.println(Feuchtigkeit_sensor.name);
    Serial.print  ("Driver Ver:   "); Serial.println(Feuchtigkeit_sensor.version);
    Serial.print  ("Unique ID:    "); Serial.println(Feuchtigkeit_sensor.sensor_id);
    Serial.print  ("Max Value:    "); Serial.print(Feuchtigkeit_sensor.max_value); Serial.println("%");
    Serial.print  ("Min Value:    "); Serial.print(Feuchtigkeit_sensor.min_value); Serial.println("%");
    Serial.print  ("Resolution:   "); Serial.print(Feuchtigkeit_sensor.resolution); Serial.println("%");  
    Serial.println("------------------------------------");

    delayMS = Feuchtigkeit_sensor.min_delay / 1000;
}

void reverse(char *str, int len)
{
    int i = 0, j = len - 1, temp;
    while (i < j)
    {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
    }
}

// Converts a given integer x to string str[].  d is the number
// of digits required in output. If d is more than the number
// of digits in x, then 0s are added at the beginning.
int intToStr(int x, char str[], int d)
{
    int i = 0;
    while (x)
    {
        str[i++] = (x % 10) + '0';
        x = x / 10;
    }

    // If number of digits required is more, then
    // add 0s at the beginning
    while (i < d)
        str[i++] = '0';

    reverse(str, i);
    str[i] = '\0';
    return i;
}
// Converts a floating point number to string.
void ftoa(float n, char *res, int afterpoint)
{
    // Extract integer part
    int ipart = (int)n;

    // Extract floating part
    float fpart = n - (float)ipart;

    // convert integer part to string
    int i = intToStr(ipart, res, 0);

    // check for display option after point
    if (afterpoint != 0)
    {
        res[i] = '.'; // add dot

        // Get the value of fraction part upto given no.
        // of points after dot. The third parameter is needed
        // to handle cases like 233.007
        fpart = fpart * pow(10, afterpoint);

        intToStr((int)fpart, res + i + 1, afterpoint);
    }
}

void reconnect()
{
    while (!client.connected())
    {
        Serial.print("Reconnecting...");

        if (!client.connect(clientId))
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" retrying in 5 seconds");
            delay(5000);
        }
    }
}

void stelleSchwarz(Adafruit_NeoPixel &stick, int start)
{
    for(int i = start; i<numPixels;i++)
    {
        stick.setPixelColor(i,0);
    }
}

void lassLEDLeuchten(Adafruit_NeoPixel &stick, int start, uint32_t farbe)
{
    stick.setPixelColor(start, farbe);

    if (start < numPixels - 1)
    {
        stelleSchwarz(stick, start + 1);
    }
}

void zeigeTemperaturAufLED(Adafruit_NeoPixel &stick, float aktuelleTemperatur)
{

    if (aktuelleTemperatur >= 14.0)
    {
        lassLEDLeuchten(stick,0, blau);
    }
    if (aktuelleTemperatur >= 16.0)
    {
        lassLEDLeuchten(stick,1, blau);
    }
    if (aktuelleTemperatur >= 18.0)
    {
        lassLEDLeuchten(stick,2, gelb);
    }
    if (aktuelleTemperatur >= 20.0)
    {
        lassLEDLeuchten(stick, 3, gelb);
    }
    if (aktuelleTemperatur >= 22.0)
    {
        lassLEDLeuchten(stick, 4, orange);
    }
    if (aktuelleTemperatur >= 24.0)
    {
        lassLEDLeuchten(stick, 5, orange);
    }
    if (aktuelleTemperatur >= 26.0)
    {
        lassLEDLeuchten(stick, 6, rot);
    }
    if (aktuelleTemperatur >= 28.0)
    {
        lassLEDLeuchten(stick, 7, rot);
    }
    stick.show();
}

void zeigeFeuchtigkeitAufLED(Adafruit_NeoPixel &stick, float aktuelleLuftfechtigkeit)
{
        if (aktuelleLuftfechtigkeit >= 10.0)
    {
        lassLEDLeuchten(stick,0, blau);
    }
    if (aktuelleLuftfechtigkeit >= 20.0)
    {
        lassLEDLeuchten(stick,1, blau);
    }
    if (aktuelleLuftfechtigkeit >= 30.0)
    {
        lassLEDLeuchten(stick,2, blau);
    }
    if (aktuelleLuftfechtigkeit >= 40.0)
    {
        lassLEDLeuchten(stick, 3, blau);
    }
    if (aktuelleLuftfechtigkeit >= 50.0)
    {
        lassLEDLeuchten(stick, 4, blau);
    }
    if (aktuelleLuftfechtigkeit >= 60.0)
    {
        lassLEDLeuchten(stick, 5, blau);
    }
    if (aktuelleLuftfechtigkeit >= 70.0)
    {
        lassLEDLeuchten(stick, 6, blau);
    }
    if (aktuelleLuftfechtigkeit >= 80.0)
    {
        lassLEDLeuchten(stick, 7, blau);
    }
    stick.show();
}


void loop()
{  
    delay(delayMS);
    int fenster = digitalRead(Fenster_Eingang);

    reconnect();
    client.loop();

    //test ob fenster zu oder auf ist
    if (fenster == 1 && FensterOffen == true)
    {
        Serial.println("Fenster wurde geschlossen");
        client.publish(MQTTNachrichtFenster.c_str(), "Fenster zu");
        FensterOffen = false;
        digitalWrite(pixelWarnleuchte, LOW);
    }
    else if (fenster != 1 && FensterOffen == false)
    {
        Serial.println("Fenster wurde geöffnet");
        client.publish(MQTTNachrichtFenster.c_str(), "Fenster offen");
        FensterOffen = true;
        digitalWrite(pixelWarnleuchte, HIGH);
    }

    //Temperaturmessungen
    sensors_event_t event_temperatur;
    dht.temperature().getEvent(&event_temperatur);

    if (isnan(event_temperatur.temperature))
    {
        Serial.println("Error reading temperature!");
    }
    else
    {
        Serial.print("Temperature: ");
        Serial.print(event_temperatur.temperature);
        Serial.println(" *C");
    }

    // Feuchtigkeitmessungen
    sensors_event_t event_feuchtigkeit;
    dht.humidity().getEvent(&event_feuchtigkeit);

    if (isnan(event_feuchtigkeit.relative_humidity))
    {
        Serial.println("Error reading humidity!");
    }
    else
    {
        Serial.print("Humidity: ");
        Serial.print(event_feuchtigkeit.relative_humidity);
        Serial.println("%");
    }

    //MQTT Nachrichten nur bei 2 Grad abweichung
    if (aktuelleTemperatur >= event_temperatur.temperature + 2.0 || aktuelleTemperatur <= event_temperatur.temperature - 2.0)
    {
        aktuelleTemperatur = event_temperatur.temperature;
        ftoa(aktuelleTemperatur, Messwert, 2);
        client.publish(MQTTNachrichtTemperatur.c_str(), Messwert);       
        Serial.println("Temperatur um 2 grad verändert");
        Serial.println(Messwert);
    }
    //MQTT Nachricht nur bei 5% Feuchtigkeit abweichung
    if (aktuelleLuftfechtigkeit >= event_feuchtigkeit.relative_humidity + 5.0 || aktuelleLuftfechtigkeit <= event_feuchtigkeit.relative_humidity - 5.0)
    {
        aktuelleLuftfechtigkeit = event_feuchtigkeit.relative_humidity;
        ftoa(aktuelleLuftfechtigkeit, Messwert, 2);
        client.publish(MQTTNachrichtFeuchtigkeit.c_str(), Messwert);
        Serial.println("Luftfeuchtigkeit um 5% geändert");
        Serial.println(Messwert);        
    }
    zeigeTemperaturAufLED(stick_Temperatur, aktuelleTemperatur);
    zeigeFeuchtigkeitAufLED(stick_Feuchtigkeit, aktuelleLuftfechtigkeit);

}