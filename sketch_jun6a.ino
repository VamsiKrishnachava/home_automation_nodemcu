#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>

// === Relay Pins ===
#define LIGHT_PIN D1
#define FAN_PIN D2
#define GEYSER_PIN D3
#define CHARGER_PIN D4

// === DHT Sensor ===
#define DHTPIN D5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// === Wi-Fi SoftAP ===
const char* ssid = "device1";
const char* password = "12345678";
ESP8266WebServer server(80);

// === State Variables ===
bool fanState = false;
bool lightState = false;
bool geyserState = false;
bool chargerState = false;
String mode = "None";
int temperature = 0;

// === Relay Update ===
void updateRelays() {
  if (mode == "Day") {
    fanState = temperature > 30;
    lightState = false;
  } else if (mode == "Night") {
    fanState = temperature > 30;
    lightState = true;
  } else if (mode == "None") {
    // Manual mode: don't auto-change
  }

  digitalWrite(FAN_PIN, fanState ? LOW : HIGH);
  digitalWrite(LIGHT_PIN, lightState ? LOW : HIGH);
  digitalWrite(GEYSER_PIN, geyserState ? LOW : HIGH);
  digitalWrite(CHARGER_PIN, chargerState ? LOW : HIGH);
}

// === Button HTML with current state ===
String getButtonHTML(String label, String endpoint, bool state) {
  String name = label + (state ? " ON" : " OFF");
  String color = state ? "#4CAF50" : "#f44336";
  return "<form action='" + endpoint + "' method='POST'>"
         "<button class='btn' style='background-color:" + color + ";'>" + name + "</button></form>";
}

// === Mode Buttons HTML ===
String getModeButtonsHTML() {
  return "<form action='/mode' method='POST'>"
         "<button class='mode' name='set' value='None'>None</button>"
         "<button class='mode' name='set' value='Day'>Day</button>"
         "<button class='mode' name='set' value='Night'>Night</button>"
         "</form>";
}

// === Page Handler ===
void handleRoot() {
  temperature = dht.readTemperature();
  if (isnan(temperature)) {
    temperature = 0;
  }

  updateRelays();

  String html = "<!DOCTYPE html><html><head><title>Home Automation</title>"
                "<meta name='viewport' content='width=device-width, initial-scale=1'>"
                "<style>"
                "body{font-family:sans-serif;padding:20px;}"
                ".btn, .mode{width:100%;padding:15px;margin:10px 0;border:none;color:white;font-size:16px;}"
                ".mode{background-color:#2196F3;}"
                "h1,h3,p{margin-bottom:10px;}"
                "</style></head><body>";

  html += "<h1>Home Automation</h1>";
  html += "<p>Temperature: <strong>" + String(temperature) + " °C</strong></p>";
  html += "<p>Mode: <strong>" + mode + "</strong></p>";

  html += "<h3>Controls</h3>";
  html += getButtonHTML("Fan", "/toggleFan", fanState);
  html += getButtonHTML("Light", "/toggleLight", lightState);
  html += getButtonHTML("Geyser", "/toggleGeyser", geyserState);
  html += getButtonHTML("Charger", "/toggleCharger", chargerState);

  html += "<h3>Set Mode</h3>";
  html += getModeButtonsHTML();

  html += "</body></html>";
  server.send(200, "text/html", html);
}

// === Toggle Handlers (each sets mode to None first) ===
void toggleFan() {
  mode = "None";
  fanState = !fanState;
  server.sendHeader("Location", "/");
  server.send(303);
}
void toggleLight() {
  mode = "None";
  lightState = !lightState;
  server.sendHeader("Location", "/");
  server.send(303);
}
void toggleGeyser() {
  mode = "None";
  geyserState = !geyserState;
  server.sendHeader("Location", "/");
  server.send(303);
}
void toggleCharger() {
  mode = "None";
  chargerState = !chargerState;
  server.sendHeader("Location", "/");
  server.send(303);
}

// === Mode Setter ===
void setMode() {
  mode = server.arg("set");
  if (mode == "None") {
    // Turn everything off
    fanState = false;
    lightState = false;
    geyserState = false;
    chargerState = false;
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

// === Setup ===
void setup() {
  Serial.begin(9600);
  dht.begin();

  pinMode(FAN_PIN, OUTPUT);
  pinMode(LIGHT_PIN, OUTPUT);
  pinMode(GEYSER_PIN, OUTPUT);
  pinMode(CHARGER_PIN, OUTPUT);

  digitalWrite(FAN_PIN, HIGH);
  digitalWrite(LIGHT_PIN, HIGH);
  digitalWrite(GEYSER_PIN, HIGH);
  digitalWrite(CHARGER_PIN, HIGH);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  Serial.print("Access Point IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, handleRoot);
  server.on("/toggleFan", HTTP_POST, toggleFan);
  server.on("/toggleLight", HTTP_POST, toggleLight);
  server.on("/toggleGeyser", HTTP_POST, toggleGeyser);
  server.on("/toggleCharger", HTTP_POST, toggleCharger);
  server.on("/mode", HTTP_POST, setMode);
  server.begin();
}

// === Loop ===
void loop() {
  server.handleClient();
}
