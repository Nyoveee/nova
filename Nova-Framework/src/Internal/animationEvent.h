#pragma once

#include <string>
#include "type_alias.h"
#include "reflection.h"

class ScriptAsset;

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