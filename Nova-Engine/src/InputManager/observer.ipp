template<typename InputEvent>
Observer<InputEvent>::Observer(std::function<void(InputEvent)> pressCallback, std::function<void(InputEvent)> releaseCallback) :
	pressCallback{ std::move(pressCallback) },
	releaseCallback{ std::move(releaseCallback) }
{}

template<typename InputEvent>
void Observer<InputEvent>::notifyPress(InputEvent data) {
	if (pressCallback) pressCallback(data);
}

template<typename InputEvent>
void Observer<InputEvent>::notifyRelease(InputEvent data) {
	if (releaseCallback) releaseCallback(data);
}