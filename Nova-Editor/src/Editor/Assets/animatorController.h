#pragma once

#include <unordered_map>
#include <string>

namespace ax::NodeEditor { struct EditorContext; }

class Editor;
class Controller;
class ResourceManager;
class AssetManager;

struct Animator;

class AnimatorController {
public:
	AnimatorController(Editor& editor);

	~AnimatorController();
	AnimatorController(AnimatorController const& other)				= delete;
	AnimatorController(AnimatorController&& other)					= delete;
	AnimatorController& operator=(AnimatorController const& other)	= delete;
	AnimatorController& operator=(AnimatorController&& other)		= delete;

public:
	void update();

private:
	void displayLeftPanel(Controller& controller);
	void displayRightPanel(Animator const& animator, Controller const& controller);
	
	void displayParameterWindow(Controller& controller);
	void displaySelectedNodeProperties(Controller& controller);

	void handleDragAndDrop(Controller& controller);

private:
	Editor& editor;
	ResourceManager& resourceManager;
	AssetManager& assetManager;
	ax::NodeEditor::EditorContext* context;

	// stores the name of parameter temporarily for editing..
	std::unordered_map<std::string, std::string> temporaryNames;
};