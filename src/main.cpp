#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const int AirValue = 676;   // you need to replace this value with Value_1
const int WaterValue = 273; // you need to replace this value with Value_2
const int SensorPin = A0;
int soilMoistureValue = 0;
int soilmoisturepercent = 0;

// Wifi connection variables
#define wifi_ssid "DIGIFIBRA-cF5T"
#define wifi_password "P92sKt3FGfsy"

// MQTT connection variables
#define mqtt_server "192.168.1.43"
#define mqtt_user "root"
#define mqtt_password "orangepi.hass"

#define mqtt_topic "bonsai/soil_moisture"

// functions
void setup_wifi();
void reconnect();
bool checkBound(float newValue, float prevValue, float maxDiff);

WiFiClient espClient;
PubSubClient client(espClient);

void setup()
{
  Serial.begin(115200); // open serial port, set the baud rate to 9600 bps
  Serial.print("*********************************************");
  Serial.println("soil Moisture Sensor Test!"); // prints title with ending line break
  delay(1000);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  Serial.println("Mqtt connected to: ");
  Serial.println(mqtt_server);
  Serial.println("Setup complete.");
  Serial.print("*********************************************");
}

// Variables for loop
long lastMsg = 0;
float prevMoisture = 0;

// threshold for sending a message
float maxDiff = 1;

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000)
  {
    soilMoistureValue = analogRead(SensorPin); // put Sensor insert into soil
    Serial.println(soilMoistureValue);
    soilmoisturepercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);
    if (soilmoisturepercent > 100)
    {
      Serial.println("100 %");
    }
    else if (soilmoisturepercent < 0)
    {
      Serial.println("0 %");
    }
    else if (soilmoisturepercent >= 0 && soilmoisturepercent <= 100)
    {
      Serial.print("Humedad: ");
      Serial.print(soilmoisturepercent);
      Serial.println("%");
    }
    if (checkBound(soilmoisturepercent, prevMoisture, maxDiff))
    {
      prevMoisture = soilmoisturepercent;
      Serial.print("Publish message: ");
      Serial.println(soilmoisturepercent);
      client.publish(mqtt_topic, String(soilmoisturepercent).c_str(), true);
    }
    lastMsg = now;
  }
  delay(5000);
  Serial.println("-------------------------------------------------");
}

void setup_wifi()
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password))
    {
      Serial.println("connected");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

bool checkBound(float newValue, float prevValue, float maxDiff)
{
  return !isnan(newValue) &&
         (newValue < prevValue - maxDiff || newValue > prevValue + maxDiff);
}
