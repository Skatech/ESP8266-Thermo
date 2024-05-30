#ifndef webui_h
#define webui_h

#include <Arduino.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <ESP8266NetBIOS.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>

ESP8266WebServer webServer(80);
WebSocketsServer wsServer(81);

void onWebSocketClientConnected(uint8_t addr);
bool onWebSocketTextMessageIncoming(const String& text, uint8_t addr);

void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    if (type == WStype_DISCONNECTED) {
        Serial.printf("[%u] Disconnected!\n", num);
    }
    else if (type == WStype_CONNECTED) {
        IPAddress ip = wsServer.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
        wsServer.sendTXT(num, "Connected");
        onWebSocketClientConnected(num);
    }
    else if (type == WStype_TEXT) {
        Serial.printf("[%u] Received text: %s\n", num, payload);

        if (onWebSocketTextMessageIncoming((char*)payload, num)) {
        }
        else {
            // String echoMessage = "Received:  " + String((char*)payload);
            // wsServer.sendTXT(num, echoMessage);
            wsServer.broadcastTXT((char*)payload);
        }
    }
    else if (type == WStype_BIN) {
            Serial.printf("[%u] get binary length: %u\n", num, length);
            // hexdump(payload, length);
            // webSocket.sendBIN(num, payload, length);
    }
    else if (type == WStype_PONG) {
    }
    else {
        Serial.printf("[%u] Unsupported message type: %u\n", num, type);
        wsServer.sendTXT(num, "Unsupported message type");
    }
}

bool webSocketSendOrBroadcastText(const String& payload, uint8_t addr = 0xff) {
    String& data = const_cast<String&>(payload);
    return (addr == 0xff)
        ? wsServer.broadcastTXT(data)
        : wsServer.sendTXT(addr, data);
}

void initWebUI() {
    wsServer.onEvent(webSocketEvent);
    wsServer.begin();

    webServer.serveStatic("/", LittleFS, "/webui/", "no-cache"); //"max-age=3600" or "no-cache"
    webServer.begin();
}

void loopWebUI() {
    webServer.handleClient();
    wsServer.loop();
}

#endif