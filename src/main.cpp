#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Ticker.h>
#include <EEPROM.h>

#define RED D0
#define GREEN D1
#define BLUE D2

String ssid = "RGB Light " + WiFi.macAddress();
const char *password = "12345678";

const int batteryLowest = 665;
const int batteryHighest = 876;

ESP8266WebServer server(80);

Ticker saveTimer;

bool on = false;

struct {
    uint16_t redValue = 0;
    uint16_t greenValue = 0;
    uint16_t blueValue = 0;
} values;

long adcToPercent(long x) {
    return (x - batteryLowest) * 100 / (batteryHighest - batteryLowest);
}

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
    String response =
            String(on) + "," + values.redValue + "," + values.greenValue + "," + values.blueValue + "," +
                    adcToPercent(analogRead(0));

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

    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else { // U_FS
            type = "filesystem";
        }

        // NOTE: if updating FS this would be the place to unmount FS using FS.end()
        Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
            Serial.println("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
            Serial.println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
            Serial.println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
            Serial.println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
            Serial.println("End Failed");
        }
    });
    ArduinoOTA.begin();
    Serial.println("OTA server started");

    server.on("/", HTTP_GET, handleRoot);
    server.on("/", HTTP_POST, handleUpdate);
    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("HTTP server started");

    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());
}

void loop() {
    server.handleClient();
    ArduinoOTA.handle();
}
