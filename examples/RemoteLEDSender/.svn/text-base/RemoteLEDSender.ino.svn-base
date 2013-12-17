#include <ICSC.h>

void setup()
{
  ICSC.begin(1, 115200);
  pinMode(2, INPUT_PULLUP);
}

void loop()
{
  static unsigned char lastPress = digitalRead(2);
  
  if (lastPress != digitalRead(2)) {
    lastPress = digitalRead(2);
    
    if (lastPress == LOW) {
      ICSC.send(2, 'P', 0, NULL);
    } else {
      ICSC.send(2, 'R', 0, NULL);
    }
  }
}
