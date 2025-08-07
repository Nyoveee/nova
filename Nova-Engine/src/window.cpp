#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <iostream>

#include "window.h"
#include "./Graphics/cameraSystem.h"

namespace {
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

constexpr int constWindowWidth = 1200;
constexpr int constWindowHeight = 900;
constexpr const char* name = "Nova Engine";
constexpr bool vsync = true;
constexpr float fixedFps = 60.f;
constexpr int maxNumOfSteps = 4;

Window& Window::instance() {
	static Window window{};
	return window;
}

Window::Window() : 
	glfwWindow		{},
	deltaTime		{},
	currentFps		{},
	windowWidth		{ constWindowWidth },
	windowHeight	{ constWindowHeight }
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
	glfwSetFramebufferSizeCallback(glfwWindow, window_size_callback);
	glfwSetKeyCallback(glfwWindow, key_callback);
	glfwSetCursorPosCallback(glfwWindow, mouse_callback);
	//glfwSetScrollCallback(window, scroll_callback);
	//glfwSetMouseButtonCallback(window, mouse_button_callback);

	glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Handling VSync
	glfwSwapInterval(vsync);

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

float Window::aspectRatio() const {
	return static_cast<float>(windowWidth) / windowHeight;
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
	}

	void mouse_callback(GLFWwindow* window, double xPosIn, double yPosIn) {
		(void) window;
		
		CameraSystem& cameraSystem = CameraSystem::instance();

		static bool firstTime = true;

		if (firstTime) {
			firstTime = false;
			cameraSystem.setLastMouse(static_cast<float>(xPosIn), static_cast<float>(yPosIn));
		}
		else {
			cameraSystem.calculateEulerAngle(static_cast<float>(xPosIn), static_cast<float>(yPosIn));
		}
	}
}