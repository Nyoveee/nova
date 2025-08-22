// Manages the c# scripts for each gameobject
// Reference: https://www.codeproject.com/Articles/320761/Cplusplus-CLI-Cheat-Sheet
#pragma once
#include "Script.hxx"

#include <entt/entt.hpp>
#include <vector>

class ECS;

namespace ScriptingAPI {
	public ref class Interface
	{
	// Public facing functions for ScriptAPIManager in unmanaged code to interact with..
	public:
		static void init(ECS& ecs, const char* runtimePath);
		static void update();

		// GameObjectID should be unique and created somewhere else
		static void addGameObjectScript(System::UInt32 entityID, System::String^ scriptName);
		static void removeGameObjectScript(System::UInt32 entityID, System::String^ scriptName);
		static std::vector<std::string> getScriptNames();
	
	public:
		// This should get the Entt reference from the engine
		template<typename T>
		static T* findNativeComponent(System::UInt32 entityID) {
			entt::entity entity{ static_cast<entt::entity>(entityID) };
			if (!registry->any_of<T>(entity)) {
				throw std::runtime_error("Entity does not contain component/Component is Invalid");
			}
			return &(registry->get<T>(entity));
		}

	private:
		static entt::registry* registry;
		using Scripts = System::Collections::Generic::List<Script^>;
		static System::Collections::Generic::Dictionary<System::UInt32, Scripts^>^ gameObjectScripts;
		static System::Collections::Generic::List<System::Type^> scriptTypes;
	};
	
}

#include "ManagedTypes.hxx"