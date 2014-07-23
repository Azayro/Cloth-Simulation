#pragma once

#include <windows.h>
#include "CGObject.h"


typedef __int64 gu_time_index;
typedef __int64 gu_time_interval;
typedef double gu_seconds;


class CGFrameCounter;

class CGClock : public CGObject {

private:

	__int64					performanceFrequency;
	gu_seconds				timeRecip;

	gu_time_index			baseTime;
	gu_time_index			prevTimeIndex;
	gu_time_index			currentTimeIndex;

	gu_time_interval		deltaTime;

	gu_time_index			stopTimeIndex;
	gu_time_interval		totalStopTime;

	bool					_clockStopped;

	CGFrameCounter			*frameCounter;

public:

	// Class methods

	static gu_time_index actualTime();


	// Instance methods

	CGClock(void);
	~CGClock(void);

	void reset();

	gu_seconds actualTimeElapsed();
	gu_seconds gameTimeElapsed();
	gu_seconds gameTimeDelta();

	void start();
	void stop();

	bool clockStopped();

	void tick();

	void reportTimingData(FILE *fp);

	int framesPerSecond();
	int minimumFPS();
	int maximumFPS();
	gu_seconds averageFPS();
	gu_seconds secondsPerFrame();
	gu_seconds minimumSPF();
	gu_seconds maximumSPF();
	gu_seconds averageSPF();

private:

	void resetClockAttributes();
	void invalidateClock();
	gu_seconds convertTimeIntervalToSeconds(gu_time_interval t);
};
