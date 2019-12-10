#ifndef PERFORMANCE_TIMER_H
#define PERFORMANCE_TIMER_H

#include <windows.h>
#include <assert.h>

class Timer
{
public:

	Timer() noexcept
		:startTime(), stopTime(), deltaTime(), SecondsPerCycle(0.0f), timeSeconds(0.0f)
	{
		this->InitTimer();
		this->Reset();
	}
	Timer(const Timer&) = delete;
	Timer(Timer&&) = delete;
	Timer& operator= (const Timer&) = delete;
	Timer& operator= (Timer&&) = delete;
	~Timer() = default;

	void StartTimer() noexcept
	{
		this->startTime = this->GetTimer();
	}
	void StopTimer() noexcept
	{
		this->stopTime = this->GetTimer();
		assert(this->stopTime.QuadPart >= this->startTime.QuadPart);
		this->deltaTime.QuadPart = this->stopTime.QuadPart - this->startTime.QuadPart;
	}
	void Reset() noexcept
	{
		this->startTime.QuadPart = 0;
		this->stopTime.QuadPart = 0;
		this->deltaTime.QuadPart = 0;
	}
	float TimeInSeconds() noexcept
	{
		float floatTime;
		floatTime = static_cast<float>(this->deltaTime.QuadPart);
		floatTime *= this->SecondsPerCycle;
		return floatTime;
	}

private:

	void InitTimer() noexcept
	{
		LARGE_INTEGER Frequency;
		QueryPerformanceFrequency(&Frequency);
		this->SecondsPerCycle = 1.0f / Frequency.QuadPart;
	}
	LARGE_INTEGER GetTimer() noexcept
	{
		LARGE_INTEGER time;
		QueryPerformanceCounter(&time);
		return time;
	}

	//Data
	LARGE_INTEGER	startTime;
	LARGE_INTEGER	stopTime;
	LARGE_INTEGER	deltaTime;
	float			SecondsPerCycle;
	float			timeSeconds;
};

#endif PERFORMANCE_TIMER_H

