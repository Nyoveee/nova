#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <memory>

#include "export.h"

#include "Libraries/type_alias.h"
#include "InputManager/observer.h"

class Window;

class InputManager {
public: 
	enum class InputType {
		Press,
		Release
	};

public:
	DLL_API InputManager();

	DLL_API ~InputManager()										= default;
	DLL_API InputManager(InputManager const& other)				= delete;
	DLL_API InputManager(InputManager&& other)					= delete;
	DLL_API InputManager& operator=(InputManager const& other)	= delete;
	DLL_API InputManager& operator=(InputManager&& other)		= delete;

public:
	// Systems can call this member function to observe to a certain InputEvent, providing the input manager with a callback.
	// Whenever this InputEvent happens, the callback is involved with the InputEvent's data.

	template <typename InputEvent>
	void subscribe(std::function<void(InputEvent)> pressCallback, std::function<void(InputEvent)> releaseCallback = {});

	// Input Manager calls broadcast which notifies all observers that are observing this specific InputEvent.
	// Client can alternatively call this function directly to manually broadcast an event to all observers as well.
	template <typename InputEvent>
	void broadcast(InputEvent data, InputType inputType = InputType::Press);

public:
	// Various callback handlers from GLFW window.
	void handleMouseMovement(Window& window, double xPosIn, double yPosIn);
	void handleScroll(Window& window, double xOffset, double yOffset);
	void handleKeyboardInput(Window& window, int key, int scancode, int action, int mods);
	void handleMouseInput(Window& window, int key, int action, int mods);

private:
	std::unordered_map<EventID, std::vector<std::unique_ptr<IObserver>>> observers;
};

#include "InputManager/inputManager.ipp"