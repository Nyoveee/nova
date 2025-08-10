#include "ManagedComponents.h"
#include "ScriptingAPI.h"

ScriptingAPI::Transform_::Transform_(System::UInt32 entityID)
	:entityID{ entityID } {
}
void ScriptingAPI::Transform_::position::set(Vector3 value)
{
	glm::vec3 newValue{ value.x,value.y,value.z };
	getNativeComponent(entityID)->position = newValue;
}
ScriptingAPI::Vector3 ScriptingAPI::Transform_::position::get()
{
	glm::vec3 vec = getNativeComponent(entityID)->position;
	Vector3 result{ vec.x,vec.y,vec.z };
	return result;
}

Transform* ScriptingAPI::Transform_::getNativeComponent(System::UInt32 entityID)
{
	// Ask the scriptingAPI to find the transform component associated with this id
	return ScriptingAPI::Interface::findNativeComponent<Transform>(entityID);
}
