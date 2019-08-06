#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

#define RED D0
#define GREEN D1
#define BLUE D2

const char *ssid = "RGB Stand 01";
const char *password = "12345678";

ESP8266WebServer server(80);

int redValue = 0;
int greenValue = 0;
int blueValue = 0;

void handleRoot() {
    String response = "10,";
    response += ssid;
    response += ",";
    response += redValue;
    response += ",";
    response += greenValue;
    response += ",";
    response += blueValue;

    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Max-Age", "10000");
    server.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "*");

    server.send(200, "application/json", response);
}

void handleUpdate() {
    const String &red = server.arg(0); // read RGB arguments
    const String &green = server.arg(1);
    const String &blue = server.arg(2);

    redValue = red.toInt();
    greenValue = green.toInt();
    blueValue = blue.toInt();

    analogWrite(D0, redValue);
    analogWrite(D1, greenValue);
    analogWrite(D2, blueValue);

    Serial.println(red);
    Serial.println(green);
    Serial.println(blue);

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
}

void loop() {
    server.handleClient();
}
