#pragma once

#include <functional>

class IObserver {
public:
	IObserver() = default;
	virtual ~IObserver() = 0 {};
};

template <typename InputEvent>
class Observer : public IObserver {
public:
	Observer(std::function<void(InputEvent)> pressCallback, std::function<void(InputEvent)> releaseCallback);

public:
	void notifyPress(InputEvent data);
	void notifyRelease(InputEvent data);

private:
	std::function<void(InputEvent)> pressCallback;
	std::function<void(InputEvent)> releaseCallback;
};

template<typename InputEvent>
Observer<InputEvent>::Observer(std::function<void(InputEvent)> pressCallback, std::function<void(InputEvent)> releaseCallback) :
	pressCallback	{ std::move(pressCallback) },
	releaseCallback { std::move(releaseCallback) }
{}

template<typename InputEvent>
void Observer<InputEvent>::notifyPress(InputEvent data) {
	pressCallback(data);
}

template<typename InputEvent>
void Observer<InputEvent>::notifyRelease(InputEvent data) {
	releaseCallback(data);
}
