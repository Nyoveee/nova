#include "ManagedComponents.h"
#include "ScriptingAPI.h"

ScriptingAPI::Transform_::Transform_(System::UInt32 entityID)
	:entityID{ entityID } {
}
void ScriptingAPI::Transform_::position::set(Vector3 value)
{
	glm::vec3 newValue{ value.x,value.y,value.z };
	ScriptingAPI::findNativeComponent<Transform>(entityID)->position = newValue;
}
ScriptingAPI::Vector3 ScriptingAPI::Transform_::position::get()
{
	glm::vec3 vec = ScriptingAPI::findNativeComponent<Transform>(entityID)->position;
	Vector3 result{ vec.x,vec.y,vec.z };
	return result;
}

