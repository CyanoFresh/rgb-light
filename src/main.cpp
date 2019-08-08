#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Ticker.h>

#define RED D0
#define GREEN D1
#define BLUE D2

const char *ssid = "RGB Light 1";
const char *password = "12345678";

ESP8266WebServer server(80);

Ticker checkTimer;
Ticker blinkTimer;

uint16_t redValue = 0;
uint16_t greenValue = 0;
uint16_t blueValue = 0;

void handleRoot() {
    String response = String(redValue) + "," + greenValue + "," + blueValue;

    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Max-Age", "10000");
    server.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "*");

    server.send(200, "text/plain", response);
}

void handleUpdate() {
    redValue = server.arg(0).toInt();
    greenValue = server.arg(1).toInt();
    blueValue = server.arg(2).toInt();

    analogWrite(D0, redValue);
    analogWrite(D1, greenValue);
    analogWrite(D2, blueValue);

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

void blink() {
   digitalWrite(LED_BUILTIN, digitalRead(LED_BUILTIN) == HIGH ? LOW : HIGH);
}

void onlineCheckTick() {
    if (WiFi.softAPgetStationNum() == 0) {
        blinkTimer.attach(1, blink);
    } else {
        blinkTimer.detach();
        digitalWrite(LED_BUILTIN, HIGH);
    }
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(D0, OUTPUT);
    pinMode(D1, OUTPUT);
    pinMode(D2, OUTPUT);

    analogWrite(RED, 0);
    analogWrite(GREEN, 0);
    analogWrite(BLUE, 0);

    Serial.begin(115200);
    Serial.println();

    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);

    server.on("/", HTTP_GET, handleRoot);
    server.on("/", HTTP_POST, handleUpdate);
    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("HTTP server started");

    checkTimer.attach_ms(300, onlineCheckTick);
}

void loop() {
    server.handleClient();
}
