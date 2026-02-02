#include "Script.hxx"
#include "API/ScriptingAPI.hxx"
#include "ScriptLibrary/Extensions/ManagedTypes.hxx"

#include <msclr/marshal_cppstd.h>
// C++/cli doesn't support friend class so this is a way to make sure scripts cannot access the init update exit functions of other scripts
// These also includes exception handling for scripts


void Script::callAwake()
{
	try {
		// Call Init Function
		awake();
	}
	catch (const std::exception& e) {
		Logger::error("{}Unable to call awake(): {}", gameObject->GetNameID(), e.what());
	}
	catch (System::Exception^ e) {
		Logger::error("{}Unable to call awake(): {}", gameObject->GetNameID(), msclr::interop::marshal_as<std::string>(e->Message));
		Interface::engine->stopSimulation();
	}
}

void Script::callInit() {
	try { 
		// Call Init Function
		init(); 
		b_Initialized = true;
	}
	catch (const std::exception& e) {
		Logger::error("{}Unable to call init(): {}", gameObject->GetNameID(), e.what());
	}
	catch (System::Exception^ e) {
		Logger::error("{}Unable to call init(): {}", gameObject->GetNameID(), msclr::interop::marshal_as<std::string>(e->Message));
		Interface::engine->stopSimulation();
	}
}

void Script::callUpdate() {
	try {
		// if it's not already intialized, call awake and start
		if (!b_Initialized) {
			callAwake();
			callInit();
		}
		update();
	}
	catch (const std::exception& e) {
		Logger::error("{}Unable to call update(): {}", gameObject->GetNameID(), e.what());
	}
	catch (System::Exception^ e) {
		Logger::error("{}Unable to call update(): {}", gameObject->GetNameID(), msclr::interop::marshal_as<std::string>(e->Message));
		Interface::engine->stopSimulation();
	}
}

void Script::callFixedUpdate() {
	try { fixedUpdate(); }
	catch (const std::exception& e) {
		Logger::error("{}Unable to call fixedUpdate(): {}", gameObject->GetNameID(), e.what());
	}
	catch (System::Exception^ e) {
		Logger::error("{}Unable to call fixedUpdate(): {}", gameObject->GetNameID(), msclr::interop::marshal_as<std::string>(e->Message));
		Interface::engine->stopSimulation();
	}
}

void Script::callExit() {
	try { exit(); }
	catch (const std::exception& e) {
		Logger::error("{}Unable to call exit(): {}", gameObject->GetNameID(), e.what());
	}
	catch (System::Exception^ e) {
		Logger::error("{}Unable to call exit(): {}", gameObject->GetNameID(), msclr::interop::marshal_as<std::string>(e->Message));
		Interface::engine->stopSimulation();
	}
}

void Script::callOnCollisionEnter(unsigned otherEntityID) {
	try { 
		GameObject^ other = gcnew GameObject(otherEntityID);
		onCollisionEnter(other); 
	}
	catch (const std::exception& e) {
		Logger::error("{}Unable to call onCollisionEnter(): {}", gameObject->GetNameID(), e.what());
	}
	catch (System::Exception^ e) {
		Logger::error("{}Unable to call onCollisionEnter(): {}", gameObject->GetNameID(), msclr::interop::marshal_as<std::string>(e->Message));
		Interface::engine->stopSimulation();
	}
}

void Script::callOnCollisionExit(unsigned otherEntityID) {
	try {
		GameObject^ other = gcnew GameObject(otherEntityID);
		onCollisionExit(other);
	}
	catch (const std::exception& e) {
		Logger::error("{}Unable to call onCollisionEnter(): {}", gameObject->GetNameID(), e.what());
	}
	catch (System::Exception^ e) {
		Logger::error("{}Unable to call onCollisionEnter(): {}", gameObject->GetNameID(), msclr::interop::marshal_as<std::string>(e->Message));
		Interface::engine->stopSimulation();
	}
}

