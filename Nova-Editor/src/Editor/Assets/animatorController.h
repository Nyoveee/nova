#pragma once

#include <unordered_map>
#include <string>
#include <vector>

#include "type_alias.h"
#include "imgui_node_editor.h"

class Editor;
class Controller;
class ResourceManager;
class AssetManager;

struct Animator;

namespace ed = ax::NodeEditor;

class AnimatorController {
public:
	struct PinData {
		ControllerNodeID node;	// what node does this pin belong to?
		
		enum class Type {
			In,
			Out	
		} type;					// type of pin..
	};

	struct Pins {
		ed::PinId pinIn;
		ed::PinId pinOut;
	};

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
	void displayRightPanel(Animator& animator, Controller& controller);
	
	void displayParameterWindow(Controller& controller);
	void displaySelectedNodeProperties(Controller& controller);

	void handleDragAndDrop(Controller& controller);

private:
	void renderNodes(Controller& controller);
	void renderNodeLinks(Controller& controller);
	void handleNodeLinking(Controller& controller);

private:
	Editor& editor;
	ResourceManager& resourceManager;
	AssetManager& assetManager;
	ax::NodeEditor::EditorContext* context;

	// stores the name of parameter temporarily for editing..
	std::unordered_map<std::string, std::string> temporaryNames;

	// store metadata related to these pins..
	std::unordered_map<std::size_t, PinData> pinMetaData;

	// map nodes back to their pins..
	std::unordered_map<ControllerNodeID, Pins> nodeToPins;

	bool toCenterToStartNode;

	std::vector<ed::NodeId> selectedNodes;
	static inline const ed::NodeId startNodeId  = 1000;
	static inline const ed::PinId  startPinId   = 1001;
};