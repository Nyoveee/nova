#include "Script.hxx"
#include "API/ScriptingAPI.hxx"
#include <msclr/marshal_cppstd.h>
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
	catch (const std::exception& e) {
		Logger::error("Unable to call init(): {}", e.what());
	}
	catch (System::Exception^ e) {
		Logger::error("Unable to call init(): {}", msclr::interop::marshal_as<std::string>(e->Message));
	}
}

void Script::callUpdate() {
	try { update(); }
	catch (const std::exception& e) {
		Logger::error("Unable to call update(): {}", e.what());
	}
	catch (System::Exception^ e) {
		Logger::error("Unable to call update(): {}", msclr::interop::marshal_as<std::string>(e->Message));
	}
}

void Script::callExit() {
	try { exit(); }
	catch (const std::exception& e) {
		Logger::error("Unable to call exit(): {}", e.what());
	}
	catch (System::Exception^ e) {
		Logger::error("Unable to call exit(): {}", msclr::interop::marshal_as<std::string>(e->Message));
	}
}

