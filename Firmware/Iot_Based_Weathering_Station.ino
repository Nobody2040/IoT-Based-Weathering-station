#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include "DHT.h"
#include "AdafruitIO_WiFi.h"

// --- Wi-Fi Credentials ---
#define WIFI_SSID       "your-wifi-name"
#define WIFI_PASS       "your-wifi-password"

// --- Adafruit IO Credentials ---
#define IO_USERNAME     "your-adafruit-io-username"
#define IO_KEY          "your-adafruit-io-key"

// --- Hardware Pin Definitions ---
#define DHTPIN          4      // DHT11 1-wire data line
#define DHTTYPE         DHT11
#define RAIN_PIN        32     // Rain sensor digital output (DO) with internal pull-up

// Setup Adafruit IO Connection Wrapper
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

// Setup Cloud Feed Handlers
AdafruitIO_Feed *tempFeed     = io.feed("temperature");
AdafruitIO_Feed *humidityFeed = io.feed("humidity");
AdafruitIO_Feed *pressureFeed = io.feed("pressure");
AdafruitIO_Feed *rainFeed     = io.feed("rain");

// Hardware Objects
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;
WebServer server(80);

// Telemetry State Variables
bool bmpReady     = false;
bool dhtReady     = false;
bool isWet        = false;

float dhtTemp     = 0.0;
float dhtHumidity = 0.0;
float bmpTemp     = 0.0;
float bmpPressure = 0.0;

// Timing counters (Adafruit IO free cap is 30/min; 10s interval keeps it safe)
unsigned long lastUploadMillis = 0;
const long uploadInterval = 10000;

unsigned long previousDHTMillis = 0;
const long dhtInterval = 2000;       // DHT read interval

// Global C-string buffer for serving web pages
char pageBuffer[2048];

// Render pure HTML page using standard C sprintf formatting
void handleRoot() {
  float currentTemp = bmpReady ? bmpTemp : dhtTemp;
  const char* rainState = isWet ? "RAIN DETECTED" : "CLEAR / DRY";
  const char* rainColor = isWet ? "#f87171" : "#4ade80";

  snprintf(pageBuffer, sizeof(pageBuffer),
    "<!DOCTYPE html><html><head>"
    "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
    "<meta http-equiv='refresh' content='2'>"
    "<title>ESP32 Weather Telemetry</title>"
    "<style>"
    "body{background:#0b1329;color:#f8fafc;font-family:sans-serif;text-align:center;padding:20px;margin:0;}"
    "h1{color:#38bdf8;font-size:22px;margin-bottom:4px;}"
    "p{color:#94a3b8;font-size:12px;margin-bottom:20px;}"
    ".box{display:grid;grid-template-columns:repeat(auto-fit,minmax(180px,1fr));gap:14px;max-width:680px;margin:auto;}"
    ".card{background:#0f1c3a;border:1px solid rgba(56,189,248,0.2);border-radius:10px;padding:16px;}"
    ".val{font-size:28px;font-weight:bold;margin:8px 0 2px;}"
    ".badge{display:inline-block;padding:6px 12px;border-radius:12px;font-weight:bold;color:%s;}"
    "</style></head><body>"
    "<h1>ESP32 Microclimate Station</h1>"
    "<p>Auto-refreshes every 2 seconds</p>"
    "<div class='box'>"
    "<div class='card'><div>TEMPERATURE</div><div class='val'>%.1f &deg;C</div></div>"
    "<div class='card'><div>HUMIDITY</div><div class='val'>%.0f %% RH</div></div>"
    "<div class='card'><div>PRESSURE</div><div class='val'>%.0f hPa</div></div>"
    "<div class='card'><div>RAIN STATUS</div><div class='val badge'>%s</div></div>"
    "</div></body></html>",
    rainColor, currentTemp, dhtHumidity, bmpPressure, rainState
  );

  server.send(200, "text/html", pageBuffer);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n==========================================================");
  Serial.println("  ESP32 WEATHER STATION: NO JSON / PURE C TELEMETRY       ");
  Serial.println("==========================================================");

  // 1. Rain sensor with internal pull-up
  pinMode(RAIN_PIN, INPUT_PULLUP);

  // 2. Initialize I2C Bus for BMP180
  Wire.begin(21, 22);

  // 3. Initialize BMP180
  if (bmp.begin()) {
    bmpReady = true;
    Serial.println("[BMP180] Connected successfully at 0x77.");
  } else {
    bmpReady = false;
    Serial.println("[BMP180] FAILED to initialize! Check SDA/SCL.");
  }

  // 4. Initialize DHT11
  dht.begin();
  Serial.println("[DHT11]  Initialized on GPIO 4.");

  // 5. Connect to Adafruit IO Cloud via Hotspot
  Serial.print("[CLOUD]  Connecting to Adafruit IO");
  io.connect();

  while (io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\n[CLOUD]  Connected to Adafruit IO Cloud successfully!");

  // 6. Start Local Web Server
  server.on("/", handleRoot);
  server.begin();

  Serial.println("----------------------------------------------------------");
  Serial.print("[LOCAL SERVER] Web page available at: http://");
  Serial.println(WiFi.localIP());
  Serial.println("[PUBLIC CLOUD] Feeds available at: https://io.adafruit.com (My Feeds)");
  Serial.println("----------------------------------------------------------\n");
}

void loop() {
  // Service incoming HTTP requests
  server.handleClient();

  // Keep MQTT connection alive
  io.run();

  unsigned long currentMillis = millis();

  // Read sensors non-blockingly every 2 seconds
  if (currentMillis - previousDHTMillis >= dhtInterval || previousDHTMillis == 0) {
    previousDHTMillis = currentMillis;

    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (!isnan(h) && !isnan(t)) {
      dhtReady = true;
      dhtHumidity = h;
      dhtTemp = t;
    } else {
      dhtReady = false;
    }

    if (bmpReady) {
      bmpTemp = bmp.readTemperature();
      bmpPressure = bmp.readPressure() / 100.0F; // Convert Pa to hPa
    }

    // Active LOW: LOW = Rain detected
    isWet = (digitalRead(RAIN_PIN) == LOW);
  }

  // Publish to Adafruit IO every 10 seconds
  if (currentMillis - lastUploadMillis >= uploadInterval) {
    lastUploadMillis = currentMillis;

    float finalTemp = bmpReady ? bmpTemp : dhtTemp;

    tempFeed->save(finalTemp);
    humidityFeed->save(dhtReady ? dhtHumidity : 0.0);
    pressureFeed->save(bmpReady ? bmpPressure : 0.0);
    rainFeed->save(isWet ? 1 : 0);

    Serial.printf("[CLOUD SYNC] Temp: %.1f C | Hum: %.0f %% | Pres: %.0f hPa | Rain: %d\n",
                  finalTemp,
                  dhtReady ? dhtHumidity : 0.0,
                  bmpReady ? bmpPressure : 0.0,
                  isWet ? 1 : 0);
  }
}