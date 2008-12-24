#include <stdio.h>
#include <SDL/SDL.h>
#include "CDeltaTime.h"

 CDeltaTime::CDeltaTime(int pIdealFPS) {
 // calls the set function
 setIdealFPS(pIdealFPS);
}
void CDeltaTime::setIdealFPS(int pIdealFPS) {
 // Change FPS settings and calculates ideal time.
idealFPS  = pIdealFPS;
idealTime = 1/float(idealFPS);
clear();
}
void CDeltaTime::clear() {
 timePrevious = SDL_GetTicks();
 timeCurrent  = SDL_GetTicks();
for (int i=15; i>=0; i--) deltaTimeArray[i] = idealTime;
}
 void CDeltaTime::update() {
 // Calculate interval between frames
timePrevious = timeCurrent;
 timeCurrent  = SDL_GetTicks();
// Calculate delta interval
deltaTimeArray[deltaCurrentVector] = (float(((timeCurrent-timePrevious)*idealFPS)*0.001));


++deltaCurrentVector &= 0xF;     //++ y nunca mayor a 15
 // calculo de promedio
  deltaTime = 0.0f;
for (int i=0; i<15; i++) deltaTime+=deltaTimeArray[i];
deltaTime *= 0.0625f; // It's the same as deltaTime /= 16, but faster.
//para pIdealFPS = 60 e intervalos de 16 clics  deltaTime es aproximadamente  1
//deltaTime=1.0f;


}
