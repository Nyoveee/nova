#include "ManagedComponents.h"
#include "ScriptingAPI.h"

ScriptingAPI::Vector3 ScriptingAPI::Transform_::position::get()
{
	return Vector3{ ScriptingAPI::Interface::findNativeComponent<Transform>(entityID)->position };
}
void ScriptingAPI::Transform_::position::set(Vector3 value)
{
	ScriptingAPI::Interface::findNativeComponent<Transform>(entityID)->position = value.native();
}


