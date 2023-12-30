#ifndef network_h
#define network_h

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266NetBIOS.h>
#include "DeviceConfig.h"

bool initNetBIOS(const String& hostname) {
    if(hostname.length()) {
        return NBNS.begin(hostname.c_str());
    }
    NBNS.end();
    return true;
}

bool initNetwork(const DeviceConfig& config) {
    IPAddress ip, gw, sm, dn;
    ip.fromString(config.address);
    gw.fromString(config.gateway);
    sm.fromString(config.subnet);
    dn.fromString(config.dns);
    return WiFi.disconnect() && WiFi.mode(WIFI_STA)
        && WiFi.config(ip, gw, sm, dn) && WiFi.hostname(config.hostname)
        && initNetBIOS(config.hostname);
}

bool beginConnect(const String& ssid, const String& password) {
    if (WiFi.disconnect() && ssid.length()) {
        const auto status = WiFi.begin(ssid, password);
        return status == WL_DISCONNECTED || status == WL_CONNECTED;
    }
    return false;
}



bool watchConnection() {
    static bool connected = false;
    const auto status = WiFi.status();
    if (connected) {
        if (status != WL_CONNECTED) {
            connected = false;
            digitalWrite(LED_BUILTIN, LOW);
            Serial.println("Connection LOST");
        }
    }
    else {
        if (status == WL_CONNECTED) {
            connected = true;
            digitalWrite(LED_BUILTIN, HIGH);
            Serial.print("Connection estabilished, address: ");
            Serial.println(WiFi.localIP());
        }
    }
    return connected;
}

#endif