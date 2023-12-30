#ifndef DeviceConfig_h
#define DeviceConfig_h

#include <Arduino.h>

class DeviceConfig {
public:
    static const String OPT_SSID;
    static const String OPT_PASSWORD;
    static const String OPT_HOSTNAME;
    static const String OPT_ADDRESS;
    static const String OPT_GATEWAY;
    static const String OPT_SUBNET;
    static const String OPT_DNS;

    String ssid;
    String password;
    String hostname;

    String address;
    String gateway;
    String subnet;
    String dns;

    bool load();
    bool save();
    bool processCommand(const String& cmd);
};

#endif
