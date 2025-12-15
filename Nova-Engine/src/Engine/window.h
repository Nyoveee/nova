#pragma once

#include <glm/fwd.hpp>
#include <functional>
#include <memory>

#include "export.h"
#include "config.h"

struct GLFWwindow;
struct ImGuiContext;

class InputManager;

// indicates if whole app is in the midst of destruction.
ENGINE_DLL_API extern std::atomic<bool> engineIsDestructing;

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

	struct GameViewPort {
		int topLeftX;
		int topLeftY;
		int gameWidth;
		int gameHeight;
	};

public:
	ENGINE_DLL_API Window(const char* name, Dimension dimension, GameConfig gameConfig, Configuration config, InputManager& inputManager, Viewport viewportConfig);

	ENGINE_DLL_API ~Window();
	ENGINE_DLL_API Window(Window const& other)				= delete;
	ENGINE_DLL_API Window(Window&& other)					= delete;
	ENGINE_DLL_API Window& operator=(Window const& other)	= delete;
	ENGINE_DLL_API Window& operator=(Window&& other)		= delete;

public:
	ENGINE_DLL_API void run(std::function<void(float)> fixedUpdateFunc, std::function<void(float)> updateFunc); // runs the game loop! :)
	ENGINE_DLL_API GLFWwindow* getGLFWwindow() const;
	ENGINE_DLL_API void toggleFullScreen();
	ENGINE_DLL_API float fps() const;
	ENGINE_DLL_API float getDeltaTime() const;
	ENGINE_DLL_API float getAccumulatedTime() const;

	ENGINE_DLL_API void quit();

	// Game view port is defined as the interactable area of the game.
	// In editor it's the size of the calculated image viewport, whilst in game mode it's the size of the scaled game in our window.
	ENGINE_DLL_API void setGameViewPort(GameViewPort gameViewPort);

	ENGINE_DLL_API GameViewPort getGameViewPort() const;

	// Normalized viewport pos is defined as a ranged from [0, 1], from the bottom left to the top right of the game viewport.
	ENGINE_DLL_API glm::vec2 getNormalizedViewportPos() const;
	
	// this is really NDC position.. since we are dealing x and y.
	ENGINE_DLL_API glm::vec2 getClipSpacePos() const;

	// scaled normalized viewport based on UI projection size.
	ENGINE_DLL_API glm::vec2 getUISpacePos() const;

	ENGINE_DLL_API GameConfig const& getGameConfig() const;

	ENGINE_DLL_API void toEnableMouse(bool toEnable);

	ENGINE_DLL_API void clearAccumulatedTime();


public:
	// GLFW function callbacks are in the global scope and therefore do not have private access to Input Manager.
	InputManager&	inputManager;

	// Dimension of the actual window.
	int				windowWidth;
	int				windowHeight;

private:
	GLFWwindow*		glfwWindow;
	ImGuiContext*	imGuiContext;

	double			deltaTime;
	double			currentFps;
	double			accumulatedTime;

	// Starting position & dimension of the game view port.
	GameViewPort 	gameViewPort;
	GameConfig		gameConfig;

	bool			isFullScreen;
};