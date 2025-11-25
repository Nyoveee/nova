#pragma once
#include "Engine/ScriptingAPIManager.h"
#include <vector>

interface struct IManagedStruct {
	// Shouldn't be callable in the c# scripts since native types is used
	void AppendValueToFieldData(serialized_field_type& fieldData);
	void SetValueFromFieldData(serialized_field_type const& fieldData);
};

