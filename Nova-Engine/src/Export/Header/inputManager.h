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
	// Various callback handlers from GLFW window.
	void handleMouseMovement(Window& window, double xPosIn, double yPosIn);
	void handleMouseClick();
	void handleScroll();
	void handleKeyboardInput(Window& window, int key, int scancode, int action, int mods);

public:

	// Systems can call this member function to observe to a certain InputEvent, providing the input manager with a callback.
	// Whenever this InputEvent happens, the callback is involved with the InputEvent's data.

	// Not a must to subscribe to release if not needed.
	template <typename InputEvent>
	void subscribe(std::function<void(InputEvent)> pressCallback, std::function<void(InputEvent)> releaseCallback = {});

private:
	// Broadcast this input event to all interested observer.
	template <typename InputEvent>
	void broadcast(InputEvent data, InputType inputType);

private:
	std::unordered_map<EventID, std::vector<std::unique_ptr<IObserver>>> observers;
};

#include "InputManager/inputManager.ipp"