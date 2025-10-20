#pragma once

#include <variant>
#include "resource.h"
#include "type_alias.h"

class Model;

class Controller : public Resource {
public:
	struct Data;
	struct Node {
		ControllerNodeID index		  = NO_CONTROLLER_NODE;
		ControllerNodeID previousNode = NO_CONTROLLER_NODE;
		ControllerNodeID nextNode	  = NO_CONTROLLER_NODE;

		TypedResourceID<Model> animation = INVALID_RESOURCE_ID;

		REFLECTABLE(
			index,
			previousNode,
			nextNode,
			animation
		)
	};

	struct Parameter {
		std::string name;
		std::variant<int, float, bool, std::string> value;

		REFLECTABLE(
			name,
			value
		)
	};

public:
	FRAMEWORK_DLL_API Controller(ResourceID id, ResourceFilePath filePath, Data data);

public:
	FRAMEWORK_DLL_API std::unordered_map<ControllerNodeID, Controller::Node> const& getNodes() const;

public:
	// this is the stuff that is de/serialised for resource pipeline.
	struct Data {
		ControllerNodeID entryNode = NO_CONTROLLER_NODE;
		std::unordered_map<ControllerNodeID, Node> nodes;
		std::vector<Parameter> parameters;

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