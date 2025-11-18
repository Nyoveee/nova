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