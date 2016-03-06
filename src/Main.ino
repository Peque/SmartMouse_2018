#include "RealMouse.h"
#include "WallFollow.h"

const int startButtonPin = 0;

RealMouse mouse;
WallFollow solver;
KnownMaze kmaze(&mouse);
bool done = false;

void setup(){
  Serial.begin(9600);
  mouse.setup();

  pinMode(startButtonPin, INPUT_PULLUP);

  Serial.println("Waiting for button press");
  while (digitalRead(startButtonPin));

  Serial.println("Beginning routine.");
  solver.setup(&kmaze);
}

void loop(){
  if (!solver.isFinished()) {
    solver.stepOnce();
  }
  else {
    if (!done) Serial.println("DONE.");
    done = true;
  }
}

