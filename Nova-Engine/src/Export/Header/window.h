#pragma once

#include <functional>
#include <memory>

#include "export.h"

struct GLFWwindow;
struct ImGuiContext;

class Window {
public:
	DLL_API Window();

	DLL_API ~Window();
	DLL_API Window(Window const& other)				= delete;
	DLL_API Window(Window&& other)					= delete;
	DLL_API Window& operator=(Window const& other)	= delete;
	DLL_API Window& operator=(Window&& other)		= delete;

public:
	DLL_API void run(std::function<void(float)> fixedUpdateFunc, std::function<void(float)> updateFunc); // runs the game loop! :)
	DLL_API GLFWwindow* getGLFWwindow() const;

private:
	GLFWwindow*		glfwWindow;
	ImGuiContext*	imGuiContext;

	double		deltaTime;
	double		currentFps;

	int			windowWidth;
	int			windowHeight;
};