// Manages the c# scripts for each gameobject
// Reference: https://www.codeproject.com/Articles/320761/Cplusplus-CLI-Cheat-Sheet
#pragma once
#include "Script.h"
#include "engine.h"

namespace ScriptingAPI {
	public ref class Interface
	{
	public:
		static void init(Engine& newEngine, const char* runtimePath);
		static void update();
		// GameObjectID should be unique and created somewhere else
		static void addGameObjectScript(System::UInt32 entityID, System::String^ scriptName);
		static void removeGameObjectScript(System::UInt32 entityID, System::String^ scriptName);
		// This should get the Entt reference from the engine
		template<typename T> static T* findNativeComponent(System::UInt32 entityID) {
			entt::registry& registry{ Interface::engine->ecs.registry };
			entt::entity entity{ static_cast<entt::entity>(entityID) };
			if (!registry.any_of<T>(entity)) {
				throw std::runtime_error("Entity does not contain component/Component is Invalid");
			}

			return &(registry.get<T>(entity));
		}
	private:
		static Engine* engine;
		using Scripts = System::Collections::Generic::List<Script^>;
		static System::Collections::Generic::Dictionary<System::UInt32, Scripts^>^ gameObjectScripts;
		static System::Collections::Generic::List<System::Type^> scriptTypes;
	};
	
}

