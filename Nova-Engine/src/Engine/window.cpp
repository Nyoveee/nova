#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <sstream>
#include <stdexcept>
#include <iostream>

#include "window.h"
#include "InputManager/inputManager.h"
#include "Graphics/cameraSystem.h"
#include "Profiling.h"
#include "Logger.h"

// Definition. Should live only in 1 TU.
std::atomic<bool> engineIsDestructing = false;

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

	void window_size_callback	(GLFWwindow* window, int width, int height);
	void key_callback			(GLFWwindow* window, int key, int scancode, int action, int mods);
	void mouse_callback			(GLFWwindow* window, double xPosIn, double yPosIn);
	void mouse_button_callback	(GLFWwindow* window, int button, int action, int mods);
	void scroll_callback		(GLFWwindow* window, double xOffset, double yOffset);
}

constexpr bool vsync = false;
constexpr float fixedFps = 60.f;
constexpr int maxNumOfSteps = 4;

Window::Window(const char* name, Dimension dimension, GameConfig gameConfig, Configuration config, InputManager& inputManager, Viewport viewportConfig) :
	inputManager		{ inputManager },
	glfwWindow			{},
	deltaTime			{},
	currentFps			{},
	accumulatedTime		{},
	windowWidth			{ dimension.width },
	windowHeight		{ dimension.height },
	gameConfig			{ gameConfig },
	imGuiContext		{},
	isFullScreen		{}
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

		// set it up once.
		window_size_callback(glfwWindow, windowWidth, windowHeight);
	}

	glfwSetKeyCallback			(glfwWindow, key_callback);
	glfwSetCursorPosCallback	(glfwWindow, mouse_callback);
	glfwSetMouseButtonCallback	(glfwWindow, mouse_button_callback);
	glfwSetScrollCallback		(glfwWindow, scroll_callback);

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

	//glfwSetInputMode(glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Handling VSync
	glfwSwapInterval(vsync);

	// swap buffer once because the white window is flashing
	glClearColor(0.f, 0.f, 0.f, 0.f);
	glfwSwapBuffers(glfwWindow);

	inputManager.subscribe<ToEnableCursor>(
		[&](ToEnableCursor toEnable) {
			toEnableMouse(toEnable == ToEnableCursor::Enable);
		}
	);

	// setGameViewPort(GameViewPort{ 0.f, 0.f, dimension});
}

Window::~Window() {
	glfwDestroyWindow(glfwWindow);
	glfwTerminate();
}

void Window::run(std::function<void(float)> fixedUpdateFunc, std::function<void(float)> updateFunc) {
	double const fixedDeltaTime = 1. / fixedFps;
	double before = 0;

	while (!glfwWindowShouldClose(glfwWindow)) {
#if defined(DEBUG)
		ZoneScoped;
#endif

		// executes fixed update based on accumulated time.
		int numOfFixedSteps = 0;
		accumulatedTime += deltaTime;
		
		double after = glfwGetTime();
		deltaTime = after - before;
		before = glfwGetTime();

		currentFps = 1 / deltaTime;

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


		{
#if defined(DEBUG)
			ZoneScopedNC("glfwSwapBuffers", tracy::Color::AliceBlue);
#endif
			glfwSwapBuffers(glfwWindow);
		}
		{
#if defined(DEBUG)
			ZoneScopedNC("glfwPollEvents", tracy::Color::AliceBlue);
#endif
		
			glfwPollEvents();

		}
#if defined(DEBUG)
		FrameMark;
#endif
	}

	// game loop stops, the whole app starts destructing..
	engineIsDestructing = true;
}

void Window::quit() {
	glfwSetWindowShouldClose(glfwWindow, true);
}

GLFWwindow* Window::getGLFWwindow() const {
	return glfwWindow;
}

void Window::toggleFullScreen()
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

GameConfig const& Window::getGameConfig() const {
	return gameConfig;
}

void Window::toEnableMouse(bool toEnable) {
	glfwSetInputMode(glfwWindow, GLFW_CURSOR, toEnable ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}

void Window::clearAccumulatedTime() {
	accumulatedTime = 0.;
}

float Window::fps() const {
	return static_cast<float>(currentFps);
}

void Window::setGameViewPort(GameViewPort p_gameViewPort) {
	gameViewPort = p_gameViewPort;
}

Window::GameViewPort Window::getGameViewPort() const {
	return gameViewPort;
}

glm::vec2 Window::getNormalizedViewportPos() const {
	double xPos, yPos;
	glfwGetCursorPos(glfwWindow, &xPos, &yPos);

	// Calculate the mouse position relative to the game's viewport.
	glm::vec2 mouseRelativeToViewPort = { xPos, yPos };
	mouseRelativeToViewPort -= glm::vec2{ gameViewPort.topLeftX, gameViewPort.topLeftY };
	mouseRelativeToViewPort /= glm::vec2{ gameViewPort.gameWidth, gameViewPort.gameHeight };

	// Flip y..
	mouseRelativeToViewPort.y = 1 - mouseRelativeToViewPort.y;
	return mouseRelativeToViewPort;
}

glm::vec2 Window::getClipSpacePos() const {
	return getNormalizedViewportPos() * 2.f - 1.f;
}

glm::vec2 Window::getUISpacePos() const {
	glm::vec2 normalisedMousePos = getNormalizedViewportPos();
	return { normalisedMousePos.x * 1920.f, normalisedMousePos.y * 1080.f };
}

float Window::getDeltaTime() const {
	return static_cast<float>(deltaTime);
}

float Window::getAccumulatedTime() const {
	return static_cast<float>(accumulatedTime);
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

		if (id == 131169 || id == 131186 || id == 131185 || id == 131218 || id == 131204 || id == 7 || id == 1281 || id == 8) {
			isAnError = false;
			return;
		}

		if (id == (1282)) {
			// it's just gonna spam every game loop
			return;
		}

		std::stringstream ss;

		ss << "---------------" << std::endl;
		ss << (isAnError ? "ERROR" : "Info") << " (" << id << "): " << message << std::endl;

		// Don't need more details for info logging.
		if (!isAnError) {
			return;
		}

		switch (source)
		{
		case GL_DEBUG_SOURCE_API:             ss << "Source: API";				break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   ss << "Source: Window System";	break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: ss << "Source: Shader Compiler";	break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:     ss << "Source: Third Party";		break;
		case GL_DEBUG_SOURCE_APPLICATION:     ss << "Source: Application";		break;
		case GL_DEBUG_SOURCE_OTHER:           ss << "Source: Other";			break;
		}

		ss << '\n';

		switch (type)
		{
		case GL_DEBUG_TYPE_ERROR:               ss << "Type: Error";				break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: ss << "Type: Deprecated Behaviour";	break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  ss << "Type: Undefined Behaviour";	break;
		case GL_DEBUG_TYPE_PORTABILITY:         ss << "Type: Portability";			break;
		case GL_DEBUG_TYPE_PERFORMANCE:         ss << "Type: Performance";			break;
		case GL_DEBUG_TYPE_MARKER:              ss << "Type: Marker";				break;
		case GL_DEBUG_TYPE_PUSH_GROUP:          ss << "Type: Push Group";			break;
		case GL_DEBUG_TYPE_POP_GROUP:           ss << "Type: Pop Group";			break;
		case GL_DEBUG_TYPE_OTHER:               ss << "Type: Other";				break;
		}

		ss << '\n';

		switch (severity)
		{
		case GL_DEBUG_SEVERITY_HIGH:			ss << "Severity: high";				break;
		case GL_DEBUG_SEVERITY_MEDIUM:			ss << "Severity: medium";			break;
		case GL_DEBUG_SEVERITY_LOW:				ss << "Severity: low";				break;
		case GL_DEBUG_SEVERITY_NOTIFICATION:	ss << "Severity: notification";		break;
		}

		ss << "\n\n";
		Logger::error("{}", ss.str());
	}

	void window_size_callback(GLFWwindow*, int width, int height) {
		// Specified dimension for our game.s
		float gameWidth = static_cast<float>(g_Window->getGameConfig().gameWidth);
		float gameHeight = static_cast<float>(g_Window->getGameConfig().gameHeight);

		float ratio = 1.f;
		float imageWidth;
		float imageHeight;

		ratio = gameWidth / static_cast<float>(width);
		imageWidth = static_cast<float>(width);
		imageHeight = gameHeight / ratio;

		if (imageHeight > height) {
			ratio = imageHeight / static_cast<float>(height);
			imageWidth /= ratio;
			imageHeight = static_cast<float>(height);
		}

		float centerXOffset = (static_cast<float>(width) - imageWidth) / 2.f;
		float centerYOffset = (static_cast<float>(height) - imageHeight) / 2.f;

		Window::GameViewPort resizedGameViewPort {
			.topLeftX = static_cast<int>(centerXOffset),
			.topLeftY = static_cast<int>(centerYOffset),
			.gameWidth = static_cast<int>(imageWidth),
			.gameHeight = static_cast<int>(imageHeight)
		};

		g_Window->setGameViewPort(resizedGameViewPort);
	}

	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		(void) window;
		(void) mods;
		(void) scancode;

		assert(g_Window);
		g_Window->inputManager.handleKeyboardInput(*g_Window, key, scancode, action, mods);
	}

	void mouse_callback(GLFWwindow* window, double xPosIn, double yPosIn) {
		(void) window;
		
		assert(g_Window);
		g_Window->inputManager.handleMouseMovement(*g_Window, xPosIn, yPosIn);
	}
	
	void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
		(void) window;

		assert(g_Window);
		g_Window->inputManager.handleMouseInput(*g_Window, button, action, mods);
	}

	void scroll_callback(GLFWwindow* window, double xOffset, double yOffset) {
		(void) window;

		assert(g_Window);
		g_Window->inputManager.handleScroll(*g_Window, xOffset, yOffset);
	}
}