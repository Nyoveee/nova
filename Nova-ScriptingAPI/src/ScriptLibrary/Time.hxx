#pragma once
public ref class Time
{
public:
	// To do, intialize from engine's deltatime in scriptingapi instead
	static property float fixedDeltaTime {
		float get() { return 1 / 60.f; }
	}
};