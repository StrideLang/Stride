
#include "Timer.h"
//[[Includes]]

//[[/Includes]]

Timer t;
//[[Init Code]]

int led = 13;
float phase = 0.0;

//[[/Init Code]]

void process()
{
//[[Dsp Code]]
      Serial.print("2 second tick: millis()=");
      Serial.println(millis());
//[[/Dsp Code]]
}


void setup() {

int tickEvent = t.every(1, process);
//[[Config Code]]

  Serial.begin(9600);
  pinMode(led, OUTPUT);
//[[/Config Code]]
}

void loop() {
  t.update();
}
