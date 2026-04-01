#pragma once

#include <variant>
#include <string>

#include "resource.h"
#include "type_alias.h"
#include "animationEvent.h"

class Model;
class ScriptAsset;

class Controller : public Resource {
public:
	struct Data;
	struct Condition;
	struct Transition;
	struct Node;
	struct Parameter;

	// Defines all these structs..
	#include "controllerStructs.h"

public:
	FRAMEWORK_DLL_API Controller(ResourceID id, ResourceFilePath filePath, Data data);

public:
	FRAMEWORK_DLL_API std::unordered_map<ControllerNodeID, Controller::Node> const& getNodes() const;

public:

#if 0
	struct Node {
		ControllerNodeID			id = NO_CONTROLLER_NODE;
		TypedResourceID<Model>		animation{ INVALID_RESOURCE_ID };
		std::vector<Transition>		transitions{};
		bool						toLoop = true;
		float						blendFactor = 0.2f;
		std::string					name{};
		std::vector<AnimationEvent>	animationEvents{};

		REFLECTABLE(
			id,
			animation,
			transitions,
			toLoop,
			blendFactor,
			name,
			animationEvents
		)
	};
#endif

	// this is the stuff that is de/serialised for resource pipeline.
	struct Data {
		ControllerNodeID entryNode = ENTRY_NODE;
		std::unordered_map<ControllerNodeID, Node> nodes { 
			{ 
				ENTRY_NODE, 
				Node{ 
					.id = ENTRY_NODE,
					.animation = INVALID_RESOURCE_ID,
					.transitions = {},
					.toLoop = true,
					.blendFactor = 0.2f,
					.previousFrameToAnimate = Node::Frame::Last,
					.name = "Entry Node",
					.animationEvents = {}
				}
			}
		};

		std::vector<Parameter> parameters {};

		REFLECTABLE(
			entryNode,
			nodes,
			parameters
		)
		
	} data;
	
	REFLECTABLE(
		data
	)
};