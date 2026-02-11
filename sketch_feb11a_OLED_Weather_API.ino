#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ---------- OLED ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); 

// ---------- Wifi ----------
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// ---------- Weather API ----------
const char* city = "Your_city";
const char* apiKey = "YOUR_WEATHER_API_KEY";

// ---------- Get weather ----------
bool getWeather(float &temperature, int &humidity, String &description) {
  if (WiFi.status() != WL_CONNECTED) return false;

  HTTPClient http;
  String url = "http://api.openweathermap.org/data/2.5/weather?q=" +
               String(city) +
               "&appid=" + String(apiKey) +
               "&units=metric";

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode > 0) {
    String payload = http.getString();

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
      Serial.println("JSON parsing failed");
      http.end();
      return false;
    }

    temperature = doc["main"]["temp"];
    humidity = doc["main"]["humidity"];
    description = String((const char*)doc["weather"][0]["description"]);

    Serial.println("Temperature: " + String(temperature) + " C");
    Serial.println("Humidity: " + String(humidity) + " %");
    Serial.println("Weather: " + description);

    http.end();
    return true;
    
  } else {
    Serial.println("HTTP request failed");
    http.end();
    return false;
  }
}


void setup() {
  Serial.begin(115200);
  delay(1000);

  // Start OLED
  Wire.begin(21, 22); // ESP32 I2C pins
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("OLED not found");
    while (true);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
}

void loop() {
  float temperature = 0;
  int humidity = 0;
  String description;

  bool success = getWeather(temperature, humidity, description);

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);

  if (success) {
    display.println("Weather in " + String(city));

    display.setTextSize(2);
    display.setCursor(0, 20);
    display.print("T: ");
    display.print(temperature);
    display.println("C");

    display.setTextSize(1);
    display.setCursor(0, 50);
    display.print("H: ");
    display.print(humidity);
    display.print("%  ");
    display.println(description);

  } else {
    display.println("Weather fetch failed!");
  }

  display.display();

  delay(600000); // update every 10 minutes
}
