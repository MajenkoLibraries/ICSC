#include <ICSC.h>

ICSC icsc(Serial, 'B');
//comment out the line above and uncomment the line below to use this with ChainDuinos
//ICSC icsc(Serial, 2 ,2); // last argument specifies DE pin on ChainDuino

void setup()
{
    Serial.begin(115200);
    icsc.begin();
    icsc.registerCommand('P', &pressed);
    icsc.registerCommand('R', &released);
    pinMode(13, OUTPUT);
    digitalWrite(13, LOW);
}

void loop()
{
    icsc.process();
}

void pressed(unsigned char src, char command, unsigned char len, char *data)
{
    digitalWrite(13, HIGH);
}

void released(unsigned char src, char command, unsigned char len, char *data)
{
    digitalWrite(13, LOW);
}


