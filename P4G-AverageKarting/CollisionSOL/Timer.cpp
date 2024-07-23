#include "Timer.h"

bool Timer::TimerDelay(int seconds)
{
	int count = seconds;
	for (int i = 0; i < count; i++)
	{
		_sleep(1000);
	}
	return false;
}

float Timer::LapTimer(float dTime)
{
	timer += dTime;
	std::setprecision(2);
	return timer;
}

void Timer::ResetTimer()
{
	timer = 0;
}