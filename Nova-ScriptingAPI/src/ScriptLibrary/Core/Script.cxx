#include "Script.hxx"
#include "API/ScriptingAPI.hxx"
#include "ScriptLibrary/Extensions/ManagedTypes.hxx"

#include <msclr/marshal_cppstd.h>
// C++/cli doesn't support friend class so this is a way to make sure scripts cannot access the init update exit functions of other scripts
// These also includes exception handling for scripts
void Script::callInit() {
	try { 
		// Set GameObject Details
		gameObject = GameObject::GetReference(entityID);
		// Call Init Function
		init(); 
	}
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

void Script::callOnCollisionEnter(unsigned otherEntityID) {
	try { 
		GameObject^ other = gcnew GameObject();
		other->entityID = otherEntityID;
		other->transformReference = other->getComponent<Transform_^>();
		onCollisionEnter(other); 
	}
	catch (const std::exception& e) {
		Logger::error("Unable to call on collision enter(): {}", e.what());
	}
	catch (System::Exception^ e) {
		Logger::error("Unable to call on collision enter(): {}", msclr::interop::marshal_as<std::string>(e->Message));
	}
}

