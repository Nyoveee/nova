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

	// this is the stuff that is de/serialised for resource pipeline.
	struct Data {
		ControllerNodeID entryNode = ENTRY_NODE;
		std::unordered_map<ControllerNodeID, Node> nodes { 
			{ ENTRY_NODE, Node{ ENTRY_NODE, INVALID_RESOURCE_ID, {}, true, "Entry Node" }}
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