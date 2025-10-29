#include "inputManager.h"

template<typename InputEvent>
ObserverID InputManager::subscribe(std::function<void(InputEvent)> pressCallback, std::function<void(InputEvent)> releaseCallback) {
	// obtain an id for our new observer we will be creating.
	ObserverID observerId = currentObserverId;

	// find an id that uniquely represents an Input Event
	EventID eventId{ Family::id<InputEvent>() };
	
	// create an unique observer..
	std::unique_ptr<IObserver> ptr = std::make_unique<Observer<InputEvent>>(
		Observer<InputEvent>{
			std::move(pressCallback), std::move(releaseCallback)
		}
	);

	// record this observer associated with this event..
	observers[eventId].push_back(
		observerId
	);

	// associate this id with this observer..
	observerIds[observerId] = std::move(ptr);

	// increase the observer id counter
	currentObserverId = ObserverID{ static_cast<std::size_t>(currentObserverId) + 1 };

	return observerId;
}

template<typename InputEvent>
void InputManager::unsubscribe(ObserverID id) {
	observerIds.erase(id);

	// find an id that uniquely represents an Input Event
	EventID eventId{ Family::id<InputEvent>() };

	auto&& ids = observers[eventId];
	auto iterator = std::ranges::find(ids, id);
	
	if (iterator != ids.end()) {
		ids.erase(iterator);
	}
}

template<typename InputEvent>
void InputManager::broadcast(InputEvent data, InputType inputType) {
	EventID eventId{ Family::id<InputEvent>() };

	auto iterator = observers.find(eventId);

	if (iterator == observers.end()) {
		return;
	}

	for (ObserverID id : iterator->second) {
		auto observerIterator = observerIds.find(id);

		// recently deleted
		if (observerIterator == observerIds.end()) {
			continue;
		}

		auto&& [_, iObserver] = *observerIterator;

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
void InputManager::mapKeyBindInput(int key, KeyType type, InputEvent data, InputMod mod) {
	std::unique_ptr<IKeyBind> keyBind = std::make_unique<KeyBind<InputEvent>>(
		KeyBind<InputEvent>{
			data, mod, [&](InputEvent data, InputType inputType) { broadcast<InputEvent>(data, inputType); }
		}
	);

	GLFWInput input = { key, type };
	mappedKeyBinds[input].push_back(std::move(keyBind));
}