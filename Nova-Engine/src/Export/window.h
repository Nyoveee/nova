#pragma once

#include <functional>
#include <memory>

#include "export.h"

struct GLFWwindow;
struct ImGuiContext;

class InputManager;

// indicates if whole app is in the midst of destruction.
extern std::atomic<bool> engineIsDestructing;

class Window {
public:
	// Q: why not just use a bool? A: enums are a lot more expressive and improves readability.
	enum class Configuration {
		FullScreen,
		Maximised,	
		Restored	// retains the original size.
	};

	enum class Viewport {
		ChangeDuringResize,
		Constant
	};

	struct Dimension {
		int width;
		int height;
	};

public:
	DLL_API Window(const char* name, Dimension dimension, Configuration config, InputManager& inputManager, Viewport viewportConfig);

	DLL_API ~Window();
	DLL_API Window(Window const& other)				= delete;
	DLL_API Window(Window&& other)					= delete;
	DLL_API Window& operator=(Window const& other)	= delete;
	DLL_API Window& operator=(Window&& other)		= delete;

public:
	DLL_API void run(std::function<void(float)> fixedUpdateFunc, std::function<void(float)> updateFunc); // runs the game loop! :)
	DLL_API GLFWwindow* getGLFWwindow() const;
	DLL_API void toggleFullScreen();
	DLL_API float fps() const;

private:
	void toEnableMouse(bool toEnable);

public:
	// GLFW function callbacks are in the global scope and therefore do not have private access to Input Manager.
	InputManager&	inputManager;

private:
	GLFWwindow*		glfwWindow;
	ImGuiContext*	imGuiContext;

	double			deltaTime;
	double			currentFps;

	int				windowWidth;
	int				windowHeight;

	bool			isFullScreen;
};