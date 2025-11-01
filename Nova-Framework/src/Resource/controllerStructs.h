#pragma once

using ParameterTypes = std::variant<int, float, bool>;

/*
	A condition is something a transition has to fulfil, in order to be deemed a valid transition.
*/
struct Condition {
	std::string name;
	ParameterTypes value;

	enum class Check {
		Greater,
		Lesser,
		Equal,
		NotEqual
	} check;

	REFLECTABLE(
		name,
		value,
		check
	)
};

/*
	A transition transits from one node to other, with a set of conditions that it needs to fulfil
*/
struct Transition {
	ControllerNodeID nextNode = NO_CONTROLLER_NODE;
	std::vector<Condition> conditions;

	REFLECTABLE(
		nextNode,
		conditions
	)
};

struct AnimationEvent {
	int key;
	TypedResourceID<ScriptAsset> scriptId;
	std::string functionName;

	REFLECTABLE(
		key,
		scriptId,
		functionName
	)

	// editor runtime.. stores a copy of the key for temporary editing..
	int copyKey = -1;
};

struct Node {
	ControllerNodeID		id			= NO_CONTROLLER_NODE;
	TypedResourceID<Model>	animation	{ INVALID_RESOURCE_ID };
	std::vector<Transition> transitions	{};
	bool					toLoop		= true;

	std::string name					{};

	std::vector<AnimationEvent>	animationEvents;

	REFLECTABLE(
		id,
		animation,
		transitions,
		toLoop,
		name,
		animationEvents
	)
};

struct Parameter {
	std::string name;
	ParameterTypes value;

	REFLECTABLE(
		name,
		value
	)
};