#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdexcept>

#include "window.h"

constexpr int windowWidth = 800;
constexpr int windowHeight = 600;
constexpr const char* name = "Nova Engine";
constexpr bool vsync = true;
constexpr float fixedFps = 60.f;
constexpr int maxNumOfSteps = 4;

Window::Window() : 
	glfwWindow  {},
	deltaTime   {},
	currentFps	{}
{
	/*---
		GLFW Initialisation
	---*/
	if (!glfwInit()) {
		throw std::runtime_error("Failed to initialise GLFW.");
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	glfwWindowHint(GLFW_AUTO_ICONIFY, GL_TRUE);

	glfwWindow = glfwCreateWindow(windowWidth, windowHeight, name, NULL, NULL);

	if (!glfwWindow) {
		glfwTerminate();
		throw std::runtime_error("Failed to create window.");
	}

	glfwMakeContextCurrent(glfwWindow);

	/*--
		Center GLFW window.
	--*/
	auto videoMode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	int width, height;
	glfwGetWindowSize(glfwWindow, &width, &height);

	float topLeftX = videoMode->width / 2.f - width / 2.f;
	float topRightX = videoMode->height / 2.f - height / 2.f;
	glfwSetWindowPos(glfwWindow, static_cast<int>(topLeftX), static_cast<int>(topRightX));

	glfwMakeContextCurrent(glfwWindow);

	/*--
		Registering callbacks..
	--*/

	//glfwSetWindowFocusCallback(window, window_focus_callback);
	//glfwSetFramebufferSizeCallback(window, window_size_callback);
	//glfwSetKeyCallback(window, key_callback);
	//glfwSetCursorPosCallback(window, cursor_position_callback);
	//glfwSetScrollCallback(window, scroll_callback);
	//glfwSetMouseButtonCallback(window, mouse_button_callback);

	// Handling VSync
	glfwSwapInterval(vsync);

	/*---
		Dynamically load OpenGL
	---*/
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		throw std::runtime_error("Failed to initialise GLAD.");
	}
}

Window& Window::instance() {
	static Window window {};
	return window;
}

Window::~Window() {
	glfwDestroyWindow(glfwWindow);
	glfwTerminate();
}

void Window::run(std::function<void()> fixedUpdateFunc, std::function<void()> updateFunc) {
	double const fixedDeltaTime = 1. / fixedFps;
	double accumulatedTime = 0.;
	double before = 0;

	while (!glfwWindowShouldClose(glfwWindow)) {
		// executes fixed update based on accumulated time.
		int numOfFixedSteps = 0;
		accumulatedTime += deltaTime;

		while (accumulatedTime >= fixedDeltaTime) {
			accumulatedTime -= fixedDeltaTime;
			numOfFixedSteps++;

			fixedUpdateFunc();

			if (numOfFixedSteps >= maxNumOfSteps) {
				break;
			}
		}

		// executes a normal update
		updateFunc();

		double after = glfwGetTime();
		deltaTime = after - before;

		currentFps = 1 / deltaTime;

		// account for vsync duration in delta time calculation
		before = glfwGetTime();

		glfwSwapBuffers(glfwWindow);
		glfwPollEvents();
	}
}
