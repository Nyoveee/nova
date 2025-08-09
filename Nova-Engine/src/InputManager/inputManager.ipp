#include "inputManager.h"

template<typename InputEvent>
void InputManager::subscribe(std::function<void(InputEvent)> pressCallback, std::function<void(InputEvent)> releaseCallback) {
	EventID eventId{ Family::id<InputEvent>() };
	
	std::unique_ptr<IObserver> ptr = std::make_unique<Observer<InputEvent>>(
		Observer<InputEvent>{
			std::move(pressCallback), std::move(releaseCallback)
		}
	);

	observers[eventId].push_back(
		std::move(ptr)
	);
}

template<typename InputEvent>
void InputManager::broadcast(InputEvent data, InputType inputType) {
	EventID eventId{ Family::id<InputEvent>() };

	for (auto const& iObserver : observers[eventId]) {
		// Dynamic cast my observer interface to get my actual object..
		Observer<InputEvent>* observer = dynamic_cast<Observer<InputEvent>*>(iObserver.get());
		assert(observer && "There should be no nullptr observers in input manager.");

		switch (inputType)
		{
		case InputManager::InputType::Press:
			observer->notifyPress(data);
			break;
		case InputManager::InputType::Release:
			observer->notifyRelease(data);
			break;
		}
	}
}
