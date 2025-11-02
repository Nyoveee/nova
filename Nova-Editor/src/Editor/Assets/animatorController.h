#pragma once

#include <unordered_map>
#include <string>
#include <vector>

#include "type_alias.h"
#include "imgui_node_editor.h"

#include "controller.h"

class Editor;
class Controller;
class ResourceManager;
class AssetManager;

struct Animator;

namespace ed = ax::NodeEditor;

#if 0
template<>
struct std::hash<TransitionID> {
	std::size_t operator()(TransitionID const& transitionId) const noexcept {
		std::size_t h1 = std::hash<ControllerNodeID>{}(transitionId.node);
		std::size_t h2 = std::hash<int>{}(transitionId.index);
		return h1 ^ (h2 << 1);
	}
};

inline bool operator==(TransitionID const& lhs, TransitionID const& rhs) {
	return lhs.node == rhs.node && lhs.index == rhs.index;
}
#endif

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

	struct TransitionData {
		ControllerNodeID node; // what node does this transition belong to?
		int index;			   // what index is this transition in?
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
	void displayLeftPanel(Animator& animator, Controller& controller);
	void displayRightPanel(Animator& animator, Controller& controller);

	void displaySelectedAnimationTimeline(Animator& animator, Controller& controller);

	void displayParameterWindow(Animator& animator, Controller& controller);
	void displaySelectedNodeProperties(Animator& animator, Controller& controller);

	void handleDragAndDrop(Controller& controller);

private:
	void renderNodes(Animator& animator, Controller& controller);
	void renderNodeLinks(Controller& controller);
	void handleNodeLinking(Controller& controller);
	void handleDeletion(Controller& controller);

	void createNewCondition(Controller& controller, Controller::Transition& transition);
	void displayParameterComboBox(Controller& controller, Controller::Condition& condition);

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

	// map links to transitions..
	std::unordered_map<std::size_t, TransitionData> linksToTransition;

	std::vector<ed::NodeId> selectedNodes;
	std::vector<ed::LinkId> selectedLinks;

	// static inline const ed::NodeId startNodeId  = 1000;
	// static inline const ed::PinId  startPinId   = 1001;

	bool isFirstFrame = true;

	int selectedAnimationFrame;
};