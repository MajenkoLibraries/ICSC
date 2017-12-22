#include <ICSC.h>

ICSC icsc(Serial, 'B');

void setup()
{
    Serial.begin(115200);
    icsc.begin();
}

void loop()
{
    icsc.process();
}


