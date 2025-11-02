#include "prefabManager.h"
#include "Serialisation/serialisation.h"
#include "component.h"
#include "engine.h"

PrefabManager::PrefabManager(Engine& engine) :
	resourceManager { engine.resourceManager },
	ecsRegistry		{ engine.ecs.registry }
{}

//definition in .h

//template<typename ...Components>
//void PrefabManager::instantiatePrefab(ResourceID id, entt::registry& ecsRegistry, const char* fileName) {
//	
//	entt::entity entity;
//	//if ID is not found in the map, deserialise it from file
//	if (prefabMap.find(id) == prefabMap.end()) {
//		entity = Serialiser::deserialisePrefab(fileName, ecsRegistry, static_cast<std::size_t>(id));
//		if (entity == entt::null) {
//			return;
//		}
//		
//		([&]() {
//			auto* component = ecsRegistry.try_get<Components>(entity);
//
//			if (component) {
//				prefabRegistry.emplace<Components>(entity, *component);
//			}
//		}(), ...);
//		
//		prefabMap[id] = entity;
//	}
//
//	else {
//		entity = prefabMap[id];
//
//		([&]() {
//			auto* component = prefabRegistry.try_get<Components>(entity);
//
//			if (component) {
//				ecsRegistry.emplace<Components>(entity, *component);
//			}
//			}(), ...);
//	}
//}
