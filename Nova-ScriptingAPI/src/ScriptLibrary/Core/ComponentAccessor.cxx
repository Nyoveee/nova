#include "ComponentAccessor.hxx"
#include "API/ScriptingAPI.hxx"
// Generics behave just like a normal function, this definition is compiled through clr at compile time
// Specializations are created at runtime
// https://learn.microsoft.com/en-us/cpp/extensions/overview-of-generics-in-visual-cpp?view=msvc-170
generic<typename T> where T : IManagedComponent
T ComponentAccessor::getComponent() {
	// We try to load the component data from the entity itself
	T component = safe_cast<T>(System::Activator::CreateInstance(T::typeid));
	if (!component->LoadDetailsFromEntity(entityID))
		return T(); // Component will be garbage collected, return null
	return component;
}

generic<typename T> where T : Script
T ComponentAccessor::getScript() { return Interface::tryGetScriptReference<T>(entityID); }
