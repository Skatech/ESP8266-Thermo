#include <Arduino.h>
#include <LittleFS.h>

#include <ESP8266WiFi.h>
#include <ESP8266NetBIOS.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>

#include "SerialCommand.h"
#include "DeviceConfig.h"
#include "network.h"
#include "webui.h"

#include <OneWire.h>
#include <DallasTemperature.h>

#define DS18B20_PIN D1

SerialCommand scmd(64);
DeviceConfig config;

OneWire oneWire(DS18B20_PIN);
DallasTemperature temperatureSensor(&oneWire);

void processCommand(const String& cmd) {
    if (config.processCommand(cmd)) {
    }
    else if (cmd == "list-networks") {
        const int8_t count = WiFi.scanNetworks();
        for (int8_t n = 0; n < count; n++) {
            Serial.println(WiFi.SSID(n));
        }
    }
    else Serial.println(
        "Unknown command. Commands: list-networks, show-config, save-config, option?, option=VALUE");
    Serial.println();
}

void serialEvent() {
    if (scmd.update()) {
        processCommand(scmd.value());
        scmd.clear();
    }
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    Serial.begin(SERIAL_SPEED);
    Serial.println("\n\n\n\n\nESP8266 - Thermal Sensing\nInitializing:"); 

    Serial.print("  filesystem... ");
    if(LittleFS.begin()) {
        Serial.println("OK");
    }
    else {
        Serial.println("FAILED, SYSTEM HALTED");
        while(true);
    }

    Serial.print("  configuration... ");
    Serial.println(config.load()
        ? "OK"
        : "FAILED, defaults loaded");
    
    Serial.print("  network... ");
    if (initNetwork(config)) {
        Serial.println("OK");
    }
    else {
        Serial.println("FAILED");
        return;
    }

    Serial.print("  connecting... ");
    if (beginConnect(config.ssid, config.password)) {
        Serial.println("OK");
    }
    else {
        Serial.println("FAILED");
        return;
    }

    initWebUI();
    

    temperatureSensor.setWaitForConversion(false);
}

//send temperature on connect
//broadcast text messages


float temperature = -125;
bool sendOrBroadcastTemperature(uint8_t addr = 0xff) {
    return webSocketSendOrBroadcastText(String(">t:") + temperature, addr);
}

void loop() { // exec times: mid=150us, max=31ms
    static uint32_t counter = 0;
    // const auto now = millis();


    if (temperatureSensor.getDS18Count() < 1) {
        if (counter & 0x1000UL)
            temperatureSensor.begin();
    }
    else {
        if (temperatureSensor.isConversionComplete()) { // 590ms since nowait request
            const float t = temperatureSensor.getTempCByIndex(0); // 30 ms
            if (t != temperature) {
                temperature = t;
                sendOrBroadcastTemperature();
                Serial.printf("T = %fC\n", temperature);
            }
            temperatureSensor.requestTemperatures(); // 2ms - nowait with non-parasitic supply
        }
    }

    if (watchConnection()) {
        loopWebUI();
    }

    counter++;
}
