#pragma once

#include "export.h"
#include "ScriptingAPIManager.h"

class DLL_API Engine {
public:
	void run();
private:
	ScriptingAPIManager scriptingAPIManager;
};