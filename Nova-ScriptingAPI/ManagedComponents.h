// Create the managed class versions that links with the native class versions for the scripts to use
// Must build the nova-scriptingAPI project for intellisense to show updated components for the scripts
#pragma once
#include "../Nova-Engine/src/Export/Header/component.h"
// To do, Macro to generate the managed class version of the components in component.h
// Call Entt to get the component
namespace ScriptingAPI {
	// Managed type
	public value struct Vector3 {
		Vector3(glm::vec3 native) : x{ native.x }, y{ native.y }, z{ native.z } {};
		glm::vec3 native() {
			glm::vec3 result;
			result.x = x;
			result.y = y;
			result.z = z;
			return result;
		}
		float x;
		float y;
		float z;
	};
	public interface class ManagedComponent { 
		void SetEntityID(System::UInt32 entityID); 
	};
	public value class Transform_: ManagedComponent {
	public:
		virtual void SetEntityID(System::UInt32 entityID) { this->entityID = entityID; };
		property Vector3 position
		{
			Vector3 get();
			void set(Vector3 value);
		}
	private:
		System::UInt32 entityID;
	};
}