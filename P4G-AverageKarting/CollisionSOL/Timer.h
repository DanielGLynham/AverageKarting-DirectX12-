#ifndef TIMER_H
#define TIMER_H

#include <math.h>
#include <time.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <iomanip>

class Timer
{


public:
	// input second timer returns bool 
	bool TimerDelay(int seconds);
	float LapTimer(float dTime);
	void ResetTimer();

private:
	float timer;

};

#endif // !TIMER_H