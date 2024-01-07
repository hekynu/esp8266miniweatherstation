#include <Wire.h>
#include <Adafruit_AM2320.h>
#include <BH1750.h>
#include <Adafruit_BMP085.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>

Adafruit_AM2320 am2320;
BH1750 lightSensor;
Adafruit_BMP085 bmp;
Adafruit_SSD1306 display(128, 64, &Wire, -1);

const int redPin = D5;
const int greenPin = D6;
const int bluePin = D7;

void setup() {
  Serial.begin(115200);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  if (!am2320.begin()) {
    Serial.println("Failed to initialize AM2320 sensor!");
    while (1);
  }

  if (!bmp.begin()) {
    Serial.println("Failed to initialize BMP180 sensor!");
    while (1);
  }

  lightSensor.begin();

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(2000);
  display.clearDisplay();
  WiFi.mode(WIFI_AP);
}

void loop() {
  delay(3000);
  float temp = am2320.readTemperature();
  float humidity = am2320.readHumidity();
  float lightLevel = lightSensor.readLightLevel();
  float pressure = bmp.readPressure() / 100.0F;

  if (!isnan(temp) && !isnan(humidity)) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    int16_t xTemp = 0;
    int16_t xHumidity = 0;
    int16_t xLight = 0;
    int16_t xPressure = 0;

    display.setCursor(xTemp, 10);
    display.print("Teplota: ");
    display.print(temp, 1);
    display.print(" C");

    display.setCursor(xHumidity, 25);
    display.print("Vlhkost: ");
    display.print(humidity, 1);
    display.print(" RH");

    display.setCursor(xLight, 40);
    display.print("Svetlo: ");
    display.print(lightLevel);
    display.print(" lux");

    display.setCursor(xPressure, 55);
    display.print("Tlak: ");
    display.print(pressure, 1);
    display.print(" hPa");

    display.display();

    // Set LED color based on temperature
    setTemperatureColor(temp);

    String newSSID = String(temp, 2) + "C " + String(humidity, 2) + "% " + String(lightLevel) + "lux " + String(pressure, 1) + "hPa";
    if (WiFi.SSID() != newSSID) {
      setWiFiNetwork(newSSID);
      Serial.print("WiFi network name changed to: ");
      Serial.println(newSSID);
    }
  } else {
    Serial.println("Failed to read data from sensors!");
  }
}

void setWiFiNetwork(String newSSID) {
  char charSSID[50];
  newSSID.toCharArray(charSSID, 50);

  uint8_t macAddr[6];
  for (int i = 0; i < 6; ++i) {
    macAddr[i] = random(0, 255);
  }

  WiFi.softAP(charSSID, "password");
  WiFi.softAPmacAddress(macAddr);
}

void setTemperatureColor(float temp) {
  int targetRed, targetGreen, targetBlue;

  if (temp >= 20.0 && temp <= 22.0) {
    targetRed = 255;
    targetGreen = 0;
    targetBlue = 0;
  } else if (temp > 22.0) {
    targetRed = 0;
    targetGreen = 255;
    targetBlue = 0;
  } else {
    targetRed = 0;
    targetGreen = 0;
    targetBlue = 255;
  }

  smoothColorTransition(targetRed, targetGreen, targetBlue);
}

void smoothColorTransition(int targetRed, int targetGreen, int targetBlue) {
  int currentRed = analogRead(redPin);
  int currentGreen = analogRead(greenPin);
  int currentBlue = analogRead(bluePin);

  int step = 10; // You can adjust this value for a faster or slower transition

  while (currentRed != targetRed || currentGreen != targetGreen || currentBlue != targetBlue) {
    if (currentRed < targetRed) {
      currentRed = min(currentRed + step, targetRed);
    } else if (currentRed > targetRed) {
      currentRed = max(currentRed - step, targetRed);
    }

    if (currentGreen < targetGreen) {
      currentGreen = min(currentGreen + step, targetGreen);
    } else if (currentGreen > targetGreen) {
      currentGreen = max(currentGreen - step, targetGreen);
    }

    if (currentBlue < targetBlue) {
      currentBlue = min(currentBlue + step, targetBlue);
    } else if (currentBlue > targetBlue) {
      currentBlue = max(currentBlue - step, targetBlue);
    }

    analogWrite(redPin, currentRed);
    analogWrite(greenPin, currentGreen);
    analogWrite(bluePin, currentBlue);

    delay(20); // You can adjust this value for a smoother or faster transition
  }
}