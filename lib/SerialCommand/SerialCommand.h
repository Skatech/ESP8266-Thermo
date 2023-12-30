#ifndef SerialCommand_h
#define SerialCommand_h

#include <Arduino.h>

class SerialCommand {
public:
    SerialCommand(int reserve);
    void clear();
    bool update();
    const String& value() const;

private:
    String _input;
    bool _ready;
};

unsigned int getCommandValueOffset(
    const String& command, const String& option, char suffix);
bool onPropertyChangeCommand(const String& command,
        String& property, const String& option);
bool onPropertyDisplayOrChangeCommand(const String& command,
        String& property, const String& option);

#endif
