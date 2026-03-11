#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// WiFi Credentials
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// IoT Server URL
const char* serverName = "http://api.thingspeak.com/update?api_key=YOUR_API_KEY";

// Temperature Sensor Pin
#define ONE_WIRE_BUS 4

// Vibration Sensor Pin
#define VIB_SENSOR 34

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

Adafruit_MPU6050 mpu;

float temperature;
float vibration;
float ax, ay, az;

void setup()
{
  Serial.begin(115200);

  // Initialize temperature sensor
  sensors.begin();

  // Initialize vibration sensor
  pinMode(VIB_SENSOR, INPUT);

  // Initialize MPU6050
  if (!mpu.begin()) 
  {
    Serial.println("MPU6050 not detected!");
    while (1);
  }

  Serial.println("MPU6050 ready");

  // Connect WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected to WiFi");
}

void loop()
{
  // Read Temperature
  sensors.requestTemperatures();
  temperature = sensors.getTempCByIndex(0);

  // Read Vibration Sensor
  vibration = analogRead(VIB_SENSOR);

  // Read Accelerometer
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  ax = a.acceleration.x;
  ay = a.acceleration.y;
  az = a.acceleration.z;

  // Print Data
  Serial.println("----- Machine Health Data -----");

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("Vibration: ");
  Serial.println(vibration);

  Serial.print("Acceleration X: ");
  Serial.println(ax);

  Serial.print("Acceleration Y: ");
  Serial.println(ay);

  Serial.print("Acceleration Z: ");
  Serial.println(az);

  // Send data to IoT server
  if(WiFi.status()== WL_CONNECTED)
  {
    HTTPClient http;

    String serverPath = String(serverName) +
                        "&field1=" + String(temperature) +
                        "&field2=" + String(vibration) +
                        "&field3=" + String(ax) +
                        "&field4=" + String(ay) +
                        "&field5=" + String(az);

    http.begin(serverPath.c_str());
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0)
    {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    }
    else
    {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  }

  delay(15000); // ThingSpeak update interval
}