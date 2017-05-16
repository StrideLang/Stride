
#include "Timer.h"
//[[Includes]]

//[[/Includes]]

Timer t;
//[[Declarations]]

int led = 13;
float phase = 0.0;

//[[/Declarations]]

void process()
{
//[[Processing]]
      Serial.print("2 second tick: millis()=");
      Serial.println(millis());
//[[/Processing]]
}


void setup() {

int tickEvent = t.every(1, process);
//[[Initialization]]

  Serial.begin(115200);
  pinMode(led, OUTPUT);
//[[/Initialization]]
}

void loop() {
  t.update();

//[[Cleanup]]
//[[/Cleanup]]
}
