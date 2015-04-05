#include <ICSC.h>

ICSC icsc(Serial, 1);

void setup()
{
    Serial.begin(115200);
    icsc.begin();
    pinMode(2, INPUT_PULLUP);
}

void loop()
{
    static unsigned char lastPress = digitalRead(2);
  
    if (lastPress != digitalRead(2)) {
        lastPress = digitalRead(2);
    
        if (lastPress == LOW) {
            icsc.send(2, 'P', 0, NULL);
        } else {
            icsc.send(2, 'R', 0, NULL);
        }
    }
}
