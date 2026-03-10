#pragma once

public delegate void Callback();

public ref class TimeoutDelegate {
public:
	TimeoutDelegate(Callback^ callback, float duration) 
		: timeElapsed	{ 0.f }
		, duration		{ duration }
		, callback		{ callback }
	{}

internal:
	float timeElapsed;
	float duration;
	Callback^ callback;
};

public ref class IntervalDelegate {
public:
	IntervalDelegate(Callback^ callback, float interval, float totalDuration)
		: totalTimeElapsed		{ 0.f }
		, intervalTimeElapsed	{ 0.f }
		, totalDuration			{ totalDuration }
		, interval				{ interval }
		, callback				{ callback }
	{}

internal:
	float totalTimeElapsed;
	float intervalTimeElapsed;

	float interval;
	float totalDuration;
	Callback^ callback;
};