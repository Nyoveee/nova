#pragma once

#include <functional>
#include <memory>

struct GLFWwindow;

class Window {
	Window();

public:
	static Window& instance();

	~Window();
	Window(Window const& other)				= delete;
	Window(Window&& other)					= delete;
	Window& operator=(Window const& other)	= delete;
	Window& operator=(Window&& other)		= delete;

public:
	void run(std::function<void()> fixedUpdateFunc, std::function<void()> updateFunc); // runs the game loop! :)

	float aspectRatio() const;

private:
	GLFWwindow* glfwWindow;
	double		deltaTime;
	double		currentFps;

	int			windowWidth;
	int			windowHeight;
};