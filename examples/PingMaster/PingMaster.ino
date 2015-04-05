#include <ICSC.h>

ICSC icsc(Serial, 1);

void pinger(unsigned char station, char command, unsigned char len, char *data)
{
  static unsigned char led = 0;
  
  digitalWrite(13, led);
  led = 1-led;
}

void setup()
{
    Serial.begin(115200);
    icsc.begin();
    pinMode(13, OUTPUT);
    icsc.registerCommand(ICSC_SYS_PONG, &pinger);
}

void loop()
{
    static unsigned long ts = millis();
  
    if (millis() - ts >= 1000) {
        ts = millis();
        icsc.send(2, ICSC_SYS_PING, 5, "PING");
    }
    icsc.process();
}
