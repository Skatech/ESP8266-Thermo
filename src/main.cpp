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
#include "temperature.h"
#include "button.h"

#define BUTTON_BUILTIN D3
#define DS18B20_PIN D1
#define HEATER_PIN D2

SerialCommand scmd(64);
DeviceConfig config;
Temperature thermal(DS18B20_PIN);
bool heating = false;
Button button(BUTTON_BUILTIN, true);

bool sendOrBroadcastTemperature(uint8_t addr = 0xff) {
    return webSocketSendOrBroadcastText(String(">t:") 
        + (thermal.isConnected() ? thermal.getTemperature() : -127.0f), addr);
}

bool sendOrBroadcastSwitchState(uint8_t addr = 0xff) {
    return webSocketSendOrBroadcastText(String(">s:")  + (uint8_t)heating, addr);
}

void onWebSocketClientConnected(uint8_t addr) {
    sendOrBroadcastTemperature(addr);
    sendOrBroadcastSwitchState(addr);
}

void switchHeating(bool on) {
    if (heating != on) {
        heating = on;
        sendOrBroadcastSwitchState();
        digitalWrite(HEATER_PIN, heating);
        Serial.print("Heater state: ");
        Serial.println(heating ? "ON" : "OFF");
    }
}

bool onWebSocketTextMessageIncoming(const String& text, uint8_t addr) {
    if(text.startsWith(">s:")) {
        bool sw = text.substring(3) == "1";
        switchHeating(sw);
        return true;
    }
    return false;
}

void processCommand(const String& cmd) {
    if (config.processCommand(cmd)) {
    }
    else if (cmd == "show-status") {
        Serial.println("Device MAC Address: " + WiFi.macAddress());
        Serial.println("Connection state: " + (WiFi.status() == WL_CONNECTED
            ? ("connected (" + WiFi.localIP().toString())
            : ("disconnected (" + String(WiFi.status()))) + ')');
        Serial.println("Temperature sensor: "
         + (thermal.isConnected()
            ? ("connected (" + String(thermal.getTemperature()) + "'C)")
            : "disconnected"));
        Serial.println(String("Heater state: ") + (heating ? "ON" : "OFF"));
    }
    else if (cmd == "list-networks") {
        const int8_t count = WiFi.scanNetworks();
        for (int8_t n = 0; n < count; n++) {
            Serial.println(WiFi.SSID(n));
        }
    }
    else Serial.println(
        "Unknown command. Commands: show-status, list-networks, show-config, save-config, option?, option=VALUE");
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

    pinMode(HEATER_PIN, OUTPUT);
    digitalWrite(HEATER_PIN, LOW);

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
}

void loop() { // exec times: mid=150us, max=31ms
    if (thermal.loop()) {
        sendOrBroadcastTemperature();
    }

    if (watchConnection()) {
        loopWebUI();
    }

    button.loop();
    if (button.pressedFor() == 20) {
        switchHeating(!heating);        
    }
}
