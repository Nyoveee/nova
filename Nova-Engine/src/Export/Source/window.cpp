#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <iostream>

#include "window.h"
#include "inputManager.h"
#include "Graphics/cameraSystem.h"

namespace {
	// i don't know any other way..
	Window* g_Window = nullptr;

	void APIENTRY glDebugOutput(
		GLenum source,
		GLenum type,
		unsigned int id,
		GLenum severity,
		GLsizei length,
		const char* message,
		const void* userParam
	);

	void window_size_callback(GLFWwindow* window, int width, int height);
	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void mouse_callback(GLFWwindow* window, double xPosIn, double yPosIn);
}

constexpr bool vsync = false;
constexpr float fixedFps = 60.f;
constexpr int maxNumOfSteps = 4;

Window::Window(const char* name, Dimension dimension, Configuration config, InputManager& inputManager, Viewport viewportConfig) :
	inputManager		{ inputManager },
	glfwWindow			{},
	deltaTime			{},
	currentFps			{},
	windowWidth			{ dimension.width },
	windowHeight		{ dimension.height },
	imGuiContext		{},
	isFullScreen		{},
	isControllingMouse	{}
{
	// Set the global variable of window to this 1 instance of Window. 
	// This is required because GLFW callbacks work in the global scope.
	// I don't know if there is any clean way for routing input callbacks to my input manager.
	g_Window = this;

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

	/*---
		Dynamically load OpenGL
	---*/
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		throw std::runtime_error("Failed to initialise GLAD.");
	}

	// Setting up debug context
	int flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
	else {
		//logger.error("Failed to initialise OpenGL's debug context.\n");
	}

	/*--
		Registering callbacks..
	--*/

	if (viewportConfig == Viewport::ChangeDuringResize) {
		glfwSetFramebufferSizeCallback(glfwWindow, window_size_callback);
	}

	glfwSetKeyCallback(glfwWindow, key_callback);
	glfwSetCursorPosCallback(glfwWindow, mouse_callback);
	//glfwSetScrollCallback(window, scroll_callback);
	//glfwSetMouseButtonCallback(window, mouse_button_callback);

	/*--
		Center GLFW window.
	--*/
	auto videoMode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	int width, height;
	glfwGetWindowSize(glfwWindow, &width, &height);

	float topLeftX = videoMode->width / 2.f - width / 2.f;
	float topRightX = videoMode->height / 2.f - height / 2.f;
	glfwSetWindowPos(glfwWindow, static_cast<int>(topLeftX), static_cast<int>(topRightX));

	switch (config)
	{
	case Window::Configuration::FullScreen:
		toggleFullScreen();
		break;
	case Window::Configuration::Maximised:
		glfwMaximizeWindow(glfwWindow);
		break;
	case Window::Configuration::Restored:
	default:
		break;
	}

	glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Handling VSync
	glfwSwapInterval(vsync);

	// swap buffer once because the white window is flashing
	glClearColor(0.f, 0.f, 0.f, 0.f);
	glfwSwapBuffers(glfwWindow);

	// subscribe to input manager
	inputManager.subscribe<ToggleCursorControl>(
		[&](ToggleCursorControl) {
			toggleMouseControl();
		}
	);
}

Window::~Window() {
	glfwDestroyWindow(glfwWindow);
	glfwTerminate();
}

void Window::run(std::function<void(float)> fixedUpdateFunc, std::function<void(float)> updateFunc) {
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

			fixedUpdateFunc(static_cast<float>(fixedDeltaTime));

			if (numOfFixedSteps >= maxNumOfSteps) {
				break;
			}
		}

		// executes a normal update
		updateFunc(static_cast<float>(deltaTime));

		double after = glfwGetTime();
		deltaTime = after - before;

		currentFps = 1 / deltaTime;

		// account for vsync duration in delta time calculation
		before = glfwGetTime();

		glfwSwapBuffers(glfwWindow);
		glfwPollEvents();
	}
}

GLFWwindow* Window::getGLFWwindow() const {
	return glfwWindow;
}

DLL_API void Window::toggleFullScreen()
{
	int windowedXPos = 0, windowedYPos = 0;
	if (!isFullScreen) glfwGetWindowPos(glfwWindow, &windowedXPos, &windowedYPos);

	GLFWmonitor* monitor = isFullScreen ? nullptr : glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	if (isFullScreen) {
		// Restore windowed mode
		glfwSetWindowAttrib(glfwWindow, GLFW_DECORATED, GLFW_TRUE);
		glfwSetWindowMonitor(glfwWindow, nullptr, windowedXPos, windowedYPos, windowWidth, windowHeight, 0);
	}
	else {
		// Switch to fullscreen mode and hide title bar
		glfwSetWindowAttrib(glfwWindow, GLFW_DECORATED, GLFW_FALSE);
		glfwSetWindowMonitor(glfwWindow, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
	}

	isFullScreen = !isFullScreen;
}

void Window::toggleMouseControl() {
	isControllingMouse = !isControllingMouse;
	glfwSetInputMode(glfwWindow, GLFW_CURSOR, isControllingMouse ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}

namespace {
	void APIENTRY glDebugOutput(
		GLenum source,
		GLenum type,
		unsigned int id,
		GLenum severity,
		GLsizei length,
		const char* message,
		const void* userParam
	) {
		// Unused parameters
		(void)length;
		(void)userParam;

		// Tip: Set a breakpoint here if you need to backtrace which function is causing the error, by viewing the stacktrace.

		bool isAnError = true;

		if (id == 131169 || id == 131185 || id == 131218 || id == 131204 || id == 7) {
			isAnError = false;
			return;
		}

		if (id == (1282)) {
			// it's just gonna spam every game loop
			return;
		}

		std::cout << "---------------" << std::endl;
		std::cout << (isAnError ? "ERROR" : "Info") << " (" << id << "): " << message << std::endl;

		// Don't need more details for info logging.
		if (!isAnError) {
			return;
		}

		switch (source)
		{
		case GL_DEBUG_SOURCE_API:             std::cout << "Source: API";				break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System";		break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler";	break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party";		break;
		case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application";		break;
		case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other";				break;
		}

		std::cout << '\n';

		switch (type)
		{
		case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error";					break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour";	break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour";	break;
		case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability";			break;
		case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance";			break;
		case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker";				break;
		case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group";			break;
		case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group";				break;
		case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other";					break;
		}

		std::cout << '\n';

		switch (severity)
		{
		case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high";					break;
		case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium";				break;
		case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low";					break;
		case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification";			break;
		}

		std::cout << "\n\n";
	}

	void window_size_callback(GLFWwindow* window, int width, int height) {
		(void) window;
		glViewport(0, 0, width, height);
	}

	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		(void) window;
		(void) mods;
		(void) scancode;

		assert(g_Window);
		g_Window->inputManager.handleKeyboardInput(*g_Window, key, scancode, action, mods);
#if 0
		using enum CameraSystem::Movement;
		CameraSystem& cameraSystem = CameraSystem::instance();

		if (key == GLFW_KEY_W) { 
			cameraSystem.setMovement(Front, action != GLFW_RELEASE);
		}

		if (key == GLFW_KEY_A) {
			cameraSystem.setMovement(Left, action != GLFW_RELEASE);
		}

		if (key == GLFW_KEY_S) {
			cameraSystem.setMovement(Back, action != GLFW_RELEASE);
		}

		if (key == GLFW_KEY_D) {
			cameraSystem.setMovement(Right, action != GLFW_RELEASE);
		}

		if (key == GLFW_KEY_SPACE) {
			cameraSystem.setMovement(Up, action != GLFW_RELEASE);
		}

		if (key == GLFW_KEY_Z) {
			cameraSystem.setMovement(Down, action != GLFW_RELEASE);
		}

		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, true);
		}
#endif
	}

	void mouse_callback(GLFWwindow* window, double xPosIn, double yPosIn) {
		(void) window;
		
		assert(g_Window);
		g_Window->inputManager.handleMouseMovement(*g_Window, xPosIn, yPosIn);
#if 0
		CameraSystem& cameraSystem = CameraSystem::instance();

		static bool firstTime = true;

		if (firstTime) {
			firstTime = false;
			cameraSystem.setLastMouse(static_cast<float>(xPosIn), static_cast<float>(yPosIn));
		}
		else {
			cameraSystem.calculateEulerAngle(static_cast<float>(xPosIn), static_cast<float>(yPosIn));
		}
#endif
	}
}