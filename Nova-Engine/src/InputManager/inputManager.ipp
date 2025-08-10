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
		case InputType::Press:
			observer->notifyPress(data);
			break;
		case InputType::Release:
			observer->notifyRelease(data);
			break;
		}
	}
}

template<typename InputEvent>
void InputManager::mapKeyBindInput(int key, KeyType type, InputEvent data) {
	std::unique_ptr<IKeyBind> keyBind = std::make_unique<KeyBind<InputEvent>>(
		KeyBind<InputEvent>{
			data, [&](InputEvent data, InputType inputType) { broadcast<InputEvent>(data, inputType); }
		}
	);

	GLFWInput input = { key, type };
	mappedKeyBinds[input].push_back(std::move(keyBind));
}