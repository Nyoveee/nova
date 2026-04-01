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

struct Node {
	ControllerNodeID			id					= NO_CONTROLLER_NODE;
	TypedResourceID<Model>		animation			{ INVALID_RESOURCE_ID };
	std::vector<Transition>		transitions			{};
	bool						toLoop				= true;
	float						blendFactor			= 0.2f;

	enum class Frame {
		First,
		Last
	} previousFrameToAnimate = Frame::Last;

	std::string					name				{};
	std::vector<AnimationEvent>	animationEvents		{};



	REFLECTABLE(
		id,
		animation,
		transitions,
		toLoop,
		blendFactor,
		previousFrameToAnimate,
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