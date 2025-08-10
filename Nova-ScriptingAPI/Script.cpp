#include "Script.h"

ScriptingAPI::Transform_ ScriptingAPI::Script::GetTransformComponent()
{
	return Transform_(entityID);
}

void ScriptingAPI::Script::SetEntityID(System::UInt32 newEntityID)
{
	entityID = newEntityID;
}
