#include "Script.hxx"
#include "API/ScriptingAPI.hxx"
// Generics behave just like a normal function, this definition is compiled through clr at compile time
// Specializations are created at runtime
// https://learn.microsoft.com/en-us/cpp/extensions/overview-of-generics-in-visual-cpp?view=msvc-170
generic<typename T> where T : IManagedComponent
T Script::getComponent() { 
	// We try to load the component data from the entity itself
	T component = safe_cast<T>(System::Activator::CreateInstance(T::typeid)); 
	if (!component->LoadDetailsFromEntity(entityID))
		return T(); // Component will be garbage collected, return null
	return component;
}

generic<typename T> where T : Script
T Script::getScript(){ return Interface::tryGetScriptReference<T>(entityID); }

// C++/cli doesn't support friend class so this is a way to make sure scripts cannot access the init update exit functions of other scripts
// These also includes exception handling for scripts
void Script::callInit() {
	try { init(); }
	catch (System::Exception^ e) {
		System::Console::WriteLine("Unable to call init():" + e->Message);
	}
}

void Script::callUpdate() {
	try { update(); }
	catch (System::Exception^ e) {
		System::Console::WriteLine("Unable to call update():" + e->Message);
	}
}

void Script::callExit() {
	try { exit(); }
	catch (System::Exception^ e) {
		System::Console::WriteLine("Unable to call exit():" + e->Message);
	}
}

void Script::setEntityID(System::UInt32 p_entityID)
{
	entityID = p_entityID;
}

