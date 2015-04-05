#include <ICSC.h>

ICSC icsc(Serial, 2);

void setup()
{
    Serial.begin(115200);
    icsc.begin();
}

void loop()
{
    icsc.process();
}


