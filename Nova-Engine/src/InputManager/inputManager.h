#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <memory>

#include "export.h"

#include "type_alias.h"
#include "InputManager/observer.h"
#include "InputManager/keybind.h"
#include "InputManager/glfwInput.h"

class Window;

class InputManager {
public:
	ENGINE_DLL_API InputManager();

	ENGINE_DLL_API ~InputManager()										= default;
	ENGINE_DLL_API InputManager(InputManager const& other)				= delete;
	ENGINE_DLL_API InputManager(InputManager&& other)					= delete;
	ENGINE_DLL_API InputManager& operator=(InputManager const& other)	= delete;
	ENGINE_DLL_API InputManager& operator=(InputManager&& other)		= delete;

public:
	void update();
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
	// Manages all key callback handlers and properly broadcasts input event with corresponding data to all observers.
	void handleKeyInput(GLFWInput key, InputType inputType);

	template<typename InputEvent>
	void mapKeyBindInput(int key, KeyType type, InputEvent data);

	void mainKeyBindMapping();
public:
	glm::vec2 mousePosition;
	float scrollOffsetY;
private:
	// observers maps Input Event to all interested observers.
	// mappedKeyBinds maps GLFW KEY macros to a keybind containing Input Event and it's corresponding data
	std::unordered_map<EventID, std::vector<std::unique_ptr<IObserver>>>  observers;
	std::unordered_map<GLFWInput, std::vector<std::unique_ptr<IKeyBind>>> mappedKeyBinds;
};

#include "InputManager/inputManager.ipp"