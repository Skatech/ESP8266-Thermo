#ifndef temperature_h
#define temperature_h

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define TEMPERATURE_CONVERSION_TIMEOUT 750

class Temperature {
public:
    Temperature(uint8_t pin) {
        _owi.begin(pin);
        _sensor.setOneWire(&_owi);
        _sensor.setWaitForConversion(false); // with normal supply mode, this allows async temperature reading
    }

    inline bool isConnected() const {
        return _connected;
    }

    inline float getTemperature() const {
        return _temperature;
    }

    bool loop() {
        static auto lastUpdate = millis();
        const auto now = millis();

        if (_connected) {
            if (_sensor.isConversionComplete()) { // 590ms since request
                const float temp = _sensor.getTempCByIndex(0); // 30 ms
                lastUpdate = now;
                if (_temperature != temp) {
                    _temperature = temp;
                    // Serial.print("Temperature: "); Serial.print(_temperature); Serial.println("'C");
                    return true;
                }
                _sensor.requestTemperatures(); // 2ms async call
            }
            else if (now - lastUpdate > TEMPERATURE_CONVERSION_TIMEOUT) {
                _connected = false;
                // Serial.println("Temperature sensor lost!");
                return true;
            }
        }
        else if (now - lastUpdate > TEMPERATURE_CONVERSION_TIMEOUT) {
            lastUpdate = now;
            _sensor.begin();
            if (_sensor.getDS18Count() > 0) {
                _connected = true;
                _sensor.requestTemperatures();
                // Serial.println("Temperature sensor found!");
                return true;
            }
        }
        return false;
    }

private:
    OneWire _owi;
    DallasTemperature _sensor;
    float _temperature;
    bool _connected;
};

#endif