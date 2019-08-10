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

bool on = false;
uint16_t redValue = 0;
uint16_t greenValue = 0;
uint16_t blueValue = 0;

void turnOn() {
    on = true;

    analogWrite(RED, redValue);
    analogWrite(GREEN, greenValue);
    analogWrite(BLUE, blueValue);
}

void turnOff() {
    on = false;

    analogWrite(RED, 0);
    analogWrite(GREEN, 0);
    analogWrite(BLUE, 0);
}

void handleRoot() {
    String response = String(on) + "," + redValue + "," + greenValue + "," + blueValue;

    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Max-Age", "10000");
    server.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "*");

    server.send(200, "text/plain", response);
}

void handleUpdate() {
    on = server.arg(0).toInt();
    redValue = server.arg(1).toInt();
    greenValue = server.arg(2).toInt();
    blueValue = server.arg(3).toInt();

    if (on) {
        turnOn();
    } else {
        turnOff();
    }

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

void onlineCheckTick() {
    if (WiFi.softAPgetStationNum() == 0) {
        if (!blinkTimer.active()) {
            blinkTimer.attach(1, []() {
                digitalWrite(LED_BUILTIN, digitalRead(LED_BUILTIN) == HIGH ? LOW : HIGH);
            });
        }
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

    turnOff();

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
