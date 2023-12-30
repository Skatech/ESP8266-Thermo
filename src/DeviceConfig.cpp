#include <Arduino.h>
#include <ArduinoJson.h>

#include <FS.h>
#include <LittleFS.h>

#include "DeviceConfig.h"
#include "SerialCommand.h"

#define CONFIG_CAPACITY 320
#define CONFIGURATION_FILE_PATH "/config/device-config.json"

const String DeviceConfig::OPT_SSID = "ssid";
const String DeviceConfig::OPT_PASSWORD = "password";
const String DeviceConfig::OPT_HOSTNAME = "hostname";
const String DeviceConfig::OPT_ADDRESS = "address";
const String DeviceConfig::OPT_GATEWAY = "gateway";
const String DeviceConfig::OPT_SUBNET = "subnet";
const String DeviceConfig::OPT_DNS = "dns";

bool readJsonFile(const char *path, JsonDocument& doc) {
    File file = LittleFS.open(path, "r");
    if (file) {
        DeserializationError result = deserializeJson(doc, file);
        file.close();
        return result == DeserializationError::Ok;
    }
    return false;
}

bool writeJsonFile(const char *path, const JsonDocument& doc) {
    File file = LittleFS.open(path, "w");
    if (file) {
        const size_t size = serializeJson(doc, file);
        file.close();
        return size > 0;
    }
    return false;
}

bool DeviceConfig::load() {
    StaticJsonDocument<CONFIG_CAPACITY> doc;
    if (readJsonFile(CONFIGURATION_FILE_PATH, doc)) {
        ssid = doc[OPT_SSID].as<String>();
        password = doc[OPT_PASSWORD].as<String>();
        hostname = doc[OPT_HOSTNAME].as<String>();
        address = doc[OPT_ADDRESS].as<String>();
        gateway = doc[OPT_GATEWAY].as<String>();
        subnet = doc[OPT_SUBNET].as<String>();
        dns = doc[OPT_DNS].as<String>();
        return true;
    }
    else {
        ssid = "MYAP";
        password = "";
        hostname = "esp";
        address = gateway = subnet = dns = "";
    }
    return false;
}

bool DeviceConfig::save() {
    StaticJsonDocument<CONFIG_CAPACITY> doc;
    doc[OPT_SSID] = ssid;
    doc[OPT_PASSWORD] = password;
    doc[OPT_HOSTNAME] = hostname;
    doc[OPT_ADDRESS] = address;
    doc[OPT_GATEWAY] = gateway;
    doc[OPT_SUBNET] = subnet;
    doc[OPT_DNS] = dns;
    
    Serial.print(doc.memoryUsage()); Serial.print(" bytes capacity used of "); Serial.println(doc.capacity());
    String s; serializeJson(doc, s); Serial.println(s);

    return writeJsonFile(CONFIGURATION_FILE_PATH, doc);
}

bool DeviceConfig::processCommand(const String& cmd) {
    if (cmd == "show-config") {
        const auto logP = [] (const String& s, const String& v) {
            Serial.print(s); Serial.print(": "); Serial.println(v); };
        logP(OPT_SSID, ssid);
        logP(OPT_PASSWORD, password);
        logP(OPT_HOSTNAME, hostname);
        logP(OPT_ADDRESS, address);
        logP(OPT_GATEWAY, gateway);
        logP(OPT_SUBNET, subnet);
        logP(OPT_DNS, dns);
    }
    else if (cmd == "save-config") {
        if (!save()) {
            Serial.println("Configuration writing FAILED");
        };
    }
    else if (onPropertyDisplayOrChangeCommand(cmd, ssid, OPT_SSID)) {}
    else if (onPropertyDisplayOrChangeCommand(cmd, password, OPT_PASSWORD)) {}
    else if (onPropertyDisplayOrChangeCommand(cmd, hostname, OPT_HOSTNAME)) {}
    else if (onPropertyDisplayOrChangeCommand(cmd, address, OPT_ADDRESS)) {}
    else if (onPropertyDisplayOrChangeCommand(cmd, gateway, OPT_GATEWAY)) {}
    else if (onPropertyDisplayOrChangeCommand(cmd, subnet, OPT_SUBNET)) {}
    else if (onPropertyDisplayOrChangeCommand(cmd, dns, OPT_DNS)) {}
    else return false;
    return true;
}
