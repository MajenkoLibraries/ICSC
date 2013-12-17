#include <ICSC.h>

void setup()
{
  ICSC.begin(2, 115200);
  ICSC.registerCommand('P', &pressed);
  ICSC.registerCommand('R', &released);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
}

void loop()
{
  ICSC.process();
}

void pressed(unsigned char src, char command, unsigned char len, char *data)
{
  digitalWrite(13, HIGH);
}

void released(unsigned char src, char command, unsigned char len, char *data)
{
  digitalWrite(13, LOW);
}


