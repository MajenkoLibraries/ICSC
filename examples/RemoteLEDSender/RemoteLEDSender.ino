#include <ICSC.h>

ICSC icsc(Serial, 'A');

//comment out the line above and uncomment the line below to use this with ChainDuinos
//ICSC icsc(Serial, 'A' ,2); // last argument specifies DE pin on ChainDuino

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
            icsc.send('B', 'P', 0, NULL);
        } else {
            icsc.send('B', 'R', 0, NULL);
        }
    }
}
