#include <ICSC.h>

void pinger(unsigned char station, char command, unsigned char len, char *data)
{
  static unsigned char led = 0;
  
  digitalWrite(13, led);
  led = 1-led;
}

void setup()
{
  ICSC.begin(1, 115200);
  pinMode(13, OUTPUT);
  ICSC.registerCommand(ICSC_SYS_PONG, &pinger);
}

void loop()
{
  static unsigned long ts = millis();
  
  if (millis() - ts >= 1000) {
    ts = millis();
    ICSC.send(2, ICSC_SYS_PING, 5, "PING");
  }
  ICSC.process();
}
