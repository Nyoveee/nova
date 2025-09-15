#pragma once
#include "Engine/ScriptingAPIManager.h"
#include <vector>
interface struct IManagedStruct {
	// Shouldn't be callable in the c# scripts since native types is used
	void AppendNativeData(FieldData& fieldData);
};