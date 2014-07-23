#include "CGClock.h"


// Private class to track frames-per-second (which varies non-linearly) and seconds-per-frame

class CGFrameCounter : public CGObject {

private:

	int					frame;

	gu_seconds			fpsRefTimeIndex;

	unsigned long		fpsCounts;

	int					_framesPerSecond, _minimumFPS, _maximumFPS, _averageFPS;
	gu_seconds			_secondsPerFrame, _minimumSPF, _maximumSPF, _averageSPF;


public:

	CGFrameCounter()
	{
		resetCounter();
	}

	void resetCounter()
	{
		frame = 0;

		fpsRefTimeIndex = 0.0;

		fpsCounts = 0;

		_framesPerSecond = 0;
		_minimumFPS = 0;
		_maximumFPS = 0;
		_averageFPS = 0;

		_secondsPerFrame = 0.0;
		_minimumSPF = 0.0;
		_maximumSPF = 0.0;
		_averageSPF = 0.0;
	}


	void updateFrameCounterForElaspsedTime(gu_seconds gameTimeElapsed)
	{
		// Update frame counter
		frame++;

		// Check if 1 second of game time has elapsed - if so modify fps
		if (gameTimeElapsed - fpsRefTimeIndex >= 1.0)
		{
			// Get current frames per second (frame/1.0)
			_framesPerSecond = frame;

			// Get current (milli)seconds per frame
			_secondsPerFrame = 1000.0 / (gu_seconds)_framesPerSecond;

			if (fpsCounts==0)
			{
				// First iteration so initialise maximum, minimum and average fps and seconds per frame
				_minimumFPS = _maximumFPS = _averageFPS = _framesPerSecond;
				_minimumSPF = _maximumSPF = _averageSPF = _secondsPerFrame;
			}
			else
			{
				// Update maximum, minimum and average fps
				if (_framesPerSecond < _minimumFPS)
					_minimumFPS = _framesPerSecond;
				else if (_framesPerSecond > _maximumFPS)
					_maximumFPS = _framesPerSecond;

				_averageFPS += _framesPerSecond;

				// Update maximum, minimum and averse (milli)seconds per frame
				if (_secondsPerFrame < _minimumSPF)
					_minimumSPF = _secondsPerFrame;
				else if (_secondsPerFrame > _maximumSPF)
					_maximumSPF = _secondsPerFrame;

				_averageSPF += _secondsPerFrame;
			}

			// Reset frame counter for next iteration
			frame = 0;
			fpsRefTimeIndex += 1.0;

			fpsCounts++;
		}
	}

	int framesPerSecond()
	{
		return _framesPerSecond;
	}

	int minimumFPS()
	{
		return _minimumFPS;
	}

	int maximumFPS()
	{
		return _maximumFPS;
	}

	gu_seconds averageFPS()
	{
		return ((gu_seconds)_averageFPS)/(gu_seconds)fpsCounts;
	}

	gu_seconds secondsPerFrame()
	{
		return _secondsPerFrame;
	}

	gu_seconds minimumSPF()
	{
		return _minimumSPF;
	}

	gu_seconds maximumSPF()
	{
		return _maximumSPF;
	}

	gu_seconds averageSPF()
	{
		return _averageSPF/(gu_seconds)fpsCounts;
	}

};



// CGClock implementation

// Class method implementation

gu_time_index CGClock::actualTime()
{
	gu_time_index t;

	QueryPerformanceCounter((LARGE_INTEGER*)&t);

	return t;
}


// Instance method implementation

CGClock::CGClock(void)
{	
	QueryPerformanceFrequency((LARGE_INTEGER*)&performanceFrequency);
	
	if (performanceFrequency!=0)
	{
		timeRecip = 1.0 / (gu_seconds)performanceFrequency; // valid high-performance counter present

		resetClockAttributes();
		frameCounter = new CGFrameCounter();
	}
	else
	{
		timeRecip = 0.0; // high-performance counter not present - timeRecip = 0 means clock cannot be started

		invalidateClock();
	}
}


CGClock::~CGClock(void)
{
	if (frameCounter)
		frameCounter->release();
}


void CGClock::reset()
{
	resetClockAttributes();

	if (frameCounter)
		frameCounter->resetCounter();
}


gu_seconds CGClock::actualTimeElapsed()
{
	return convertTimeIntervalToSeconds(CGClock::actualTime() - baseTime);
}


gu_seconds CGClock::gameTimeElapsed()
{
	return convertTimeIntervalToSeconds((((_clockStopped) ? stopTimeIndex : CGClock::actualTime()) - baseTime) - totalStopTime);
}


gu_seconds CGClock::gameTimeDelta()
{
	return convertTimeIntervalToSeconds(deltaTime);
}


void CGClock::start()
{
	// Only allow clock to start if a valid performance frequency is present
	if (_clockStopped && performanceFrequency!=0)
	{
		gu_time_index		restartTimeIndex = CGClock::actualTime();

		totalStopTime += restartTimeIndex - stopTimeIndex;
		prevTimeIndex = restartTimeIndex;

		_clockStopped = false;
	}
}


void CGClock::stop()
{
	if (!_clockStopped)
	{
		stopTimeIndex = CGClock::actualTime();

		_clockStopped = true;
	}
}


bool CGClock::clockStopped()
{
	return _clockStopped;
}


void CGClock::tick()
{
	if (_clockStopped)
	{
		deltaTime = 0;
		return;
	}

	currentTimeIndex = CGClock::actualTime();
	deltaTime = currentTimeIndex - prevTimeIndex;
	prevTimeIndex = currentTimeIndex;

	if (frameCounter)
		frameCounter->updateFrameCounterForElaspsedTime(convertTimeIntervalToSeconds((currentTimeIndex - baseTime) - totalStopTime));
}


void CGClock::reportTimingData(FILE *fp)
{
#if 0
	FILE *temp = NULL;
	
	fopen_s(&temp, "fps.txt", "w");

	if (!temp)
		return;

	fprintf_s(temp, "Clock timing data...\n");

	fprintf_s(temp, "max FPS = %d\n", maximumFPS());
	fprintf_s(temp, "min FPS = %d\n", minimumFPS());
	fprintf_s(temp, "average FPS = %f\n", averageFPS());
	
	fprintf_s(temp, "max SPF = %f\n", maximumSPF() / 1000.0);
	fprintf_s(temp, "min SPF = %f\n", minimumSPF() / 1000.0);
	fprintf_s(temp, "average SPF = %f\n", averageSPF() / 1000.0);

	fclose(temp);

#else

	if (!fp)
		return;

	fprintf_s(fp, "max FPS = %d\n", maximumFPS());
	fprintf_s(fp, "min FPS = %d\n", minimumFPS());
	fprintf_s(fp, "average FPS = %f\n", averageFPS());
	
	fprintf_s(fp, "max SPF = %f\n", maximumSPF() / 1000.0);
	fprintf_s(fp, "min SPF = %f\n", minimumSPF() / 1000.0);
	fprintf_s(fp, "average SPF = %f\n", averageSPF() / 1000.0);

#endif

}


int CGClock::framesPerSecond()
{
	return (frameCounter) ? frameCounter->framesPerSecond() : 0;
}


int CGClock::minimumFPS()
{
	return (frameCounter) ? frameCounter->minimumFPS() : 0;
}


int CGClock::maximumFPS()
{
	return (frameCounter) ? frameCounter->maximumFPS() : 0;
}


gu_seconds CGClock::averageFPS()
{
	return (frameCounter) ? frameCounter->averageFPS() : 0.0;
}


gu_seconds CGClock::secondsPerFrame()
{
	return (frameCounter) ? frameCounter->secondsPerFrame() : 0.0;
}


gu_seconds CGClock::minimumSPF()
{
	return (frameCounter) ? frameCounter->minimumSPF() : 0.0;
}


gu_seconds CGClock::maximumSPF()
{
	return (frameCounter) ? frameCounter->maximumSPF() : 0.0;
}


gu_seconds CGClock::averageSPF()
{
	return (frameCounter) ? frameCounter->averageSPF() : 0.0;
}



// Private method implementation

void CGClock::resetClockAttributes()
{
	baseTime = CGClock::actualTime();

	prevTimeIndex = baseTime;

	deltaTime = 0;

	stopTimeIndex = 0;
	totalStopTime = 0;

	_clockStopped = false;
}


// Force clock into stopped state (only called in constructor).  The clock cannot be
// started (see start() method above)
void CGClock::invalidateClock()
{
	baseTime = 0;
	prevTimeIndex = 0;
	currentTimeIndex = 0;

	deltaTime = 0;

	stopTimeIndex = 0;
	totalStopTime = 0;

	_clockStopped = true;

	frameCounter = NULL;
}


// Convert time interval to gu_seconds
gu_seconds CGClock::convertTimeIntervalToSeconds(gu_time_interval t)
{
	return (gu_seconds)t * timeRecip;
}

