#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Ticker.h>
#include <EEPROM.h>

#define RED D0
#define GREEN D1
#define BLUE D2

const char *ssid = "RGB Light 2";
const char *password = "12345678";

ESP8266WebServer server(80);

Ticker saveTimer;

bool on = false;

struct {
    uint16_t redValue = 0;
    uint16_t greenValue = 0;
    uint16_t blueValue = 0;
} values;

void turnOn() {
    on = true;

    analogWrite(RED, values.redValue);
    analogWrite(GREEN, values.greenValue);
    analogWrite(BLUE, values.blueValue);
}

void turnOff() {
    on = false;

    analogWrite(RED, 0);
    analogWrite(GREEN, 0);
    analogWrite(BLUE, 0);
}

void handleRoot() {
    String response = String(on) + "," + values.redValue + "," + values.greenValue + "," + values.blueValue;

    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Max-Age", "10000");
    server.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "*");

    server.send(200, "text/plain", response);
}

void saveValues() {
    EEPROM.put(0, values);
    EEPROM.commit();

    Serial.println("Values saved");
    Serial.println(values.redValue);
    Serial.println(values.greenValue);
    Serial.println(values.blueValue);
}

void handleUpdate() {
    on = server.arg(0).toInt();
    values.redValue = server.arg(1).toInt();
    values.greenValue = server.arg(2).toInt();
    values.blueValue = server.arg(3).toInt();

    if (on) {
        turnOn();
    } else {
        turnOff();
    }

    saveTimer.once(2, saveValues);

    handleRoot();
}

void handleNotFound() {
    if (server.method() == HTTP_OPTIONS) {
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.sendHeader("Access-Control-Max-Age", "10000");
        server.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
        server.sendHeader("Access-Control-Allow-Headers", "*");
        server.send(204);
    } else {
        server.send(404, "text/plain", "");
    }
}

void setup() {
    pinMode(D0, OUTPUT);
    pinMode(D1, OUTPUT);
    pinMode(D2, OUTPUT);

    Serial.begin(115200);
    Serial.println();

    EEPROM.begin(sizeof(values));

    EEPROM.get(0, values);

    turnOn();

    Serial.println("Loaded values:");

    Serial.println(values.redValue);
    Serial.println(values.greenValue);
    Serial.println(values.blueValue);

    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);

    server.on("/", HTTP_GET, handleRoot);
    server.on("/", HTTP_POST, handleUpdate);
    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("HTTP server started");
}

void loop() {
    server.handleClient();
}
