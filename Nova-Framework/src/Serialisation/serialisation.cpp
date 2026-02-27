#include "serialisation.h"
#include "component.h"
#include <string>
#include <fstream>
#include <iomanip>
#include <algorithm>

namespace Serialiser {
	void Serialiser::serialiseScene(entt::registry& registry, std::vector<Layer> const& layers, const char* fileName) {

		Json js;
		std::vector<Json> jsonVec;

		for (auto&& [entity] : registry.view<entt::entity>().each()) {
			Json componentsJson = serialiseComponents<ALL_COMPONENTS>(registry, entity);
			jsonVec.push_back(componentsJson);
		}

		std::reverse(jsonVec.begin(), jsonVec.end());
		//std::sort(jsonVec.begin(), jsonVec.end(), [](Json& a, Json& b) {return a["id"] < b["id"]; });
		

		// save to output file
		js["entities"] = jsonVec;

		Json layerJson;
		
		for (auto&& layer : layers) {
			layerJson.push_back(serializeToJson(layer));
		}

		js["layers"] = layerJson;

		std::ofstream file(fileName);

		if (!file) {
			return;
		}

		file << js.dump(4) << std::endl;
		//file << std::fixed<<std::setprecision(4)<< js.dump(4) << std::endl;
	}

	void deserialiseScene(entt::registry& registry, std::vector<Layer>& layers, const char* fileName) {
		try {
			std::ifstream file(fileName);

			if (!file.is_open())
				return;

			Json j;

			j = nlohmann::ordered_json::parse(file);

			//file >> j;

			layers.clear();

			for (const auto& layerJsonElement : j["layers"]) {
				Layer layer;
				deserializeFromJson(layer, layerJsonElement);
				layers.push_back(layer);
			}

			if (layers.empty()) {
				Logger::warn("No layer deserialised. Empty constructing one because every scene requires one layer.");
				layers.push_back({ "Default", {} });
			}

			for (const auto& en : j["entities"]) {
				auto entity = registry.create(en["id"]);
				deserialiseComponents<ALL_COMPONENTS>(registry, entity, en);
			}
		}
		catch (std::exception const& ex) {
			Logger::error("Failed to deserialise scene. {}", ex.what());
		}
	}

	template <typename ...Windows>
	void serialiseEditorConfig(const char* fileName, bool console, bool debugUi, bool hierarchy, bool componentInspector) {
		std::ofstream file(fileName);

		if (!file.is_open())
			return;

		Json j;
		Json tempj;
		//([&]() {
		//	tempj[magic_enum::enum_name(Windows)] = 0;
		//	}(), ...);
		//j["Windows"] = tempj;

		file << std::setw(4) << j << std::endl;		

	}
	void deserialiseEditorConfig(const char* fileName) {
		std::ifstream file(fileName);

		if (!file.is_open())
			return;

		Json j;
		file >> j;
	}

	PrefabEntityID deserialisePrefab(const char* filepath, ResourceID prefabResourceId, entt::registry& prefabRegistry, std::unordered_map<PrefabFileEntityID, PrefabEntityID>& mapping, std::unordered_map<EntityGUID, PrefabEntityID>& entityGuidToPrefabId) {
		std::ifstream file(filepath);
		
		if (!file) {
			return entt::null;
		}

		Json jsonFile;
		file >> jsonFile;

		PrefabFileEntityID fileRootEntity = jsonFile["RootEntity"];
		PrefabEntityID prefabEntityId = deserialisePrefabRecursive(jsonFile["Entities"], fileRootEntity, prefabRegistry, prefabResourceId, mapping, entityGuidToPrefabId);
		
		if (prefabEntityId == entt::null) {
			Logger::error("Failed to deserialise {}! Please check the prefab file. Prefab not loaded.", filepath);
			return prefabEntityId;
		}

		// Remember to update entity data for our root entity!
		// At this point, we have strong guarantee that entity data exist.
		EntityData& entityData = prefabRegistry.get<EntityData>(prefabEntityId);

		// Finally, we update prefab entity's prefab metadata..
		entityData.prefabID = { prefabResourceId };

		return prefabEntityId;
	}

	void serialisePrefab(entt::registry& registry, entt::entity entity, std::ofstream& file) {
		if (!file) {
			return;
		}

		std::vector<Json> jsonVec;

		Json j;
		j["RootEntity"] = entity;
		serialisePrefabRecursive(registry, entity, jsonVec, true);
		j["Entities"] = jsonVec;
		file << std::setw(4) << j << std::endl;

		Logger::debug("Prefab Serialized");

	}

	void serialisePrefabRecursive(entt::registry& registry, entt::entity entity, std::vector<Json>& jsonVec, bool checkParent) {
		Json json = serialiseComponents<ALL_COMPONENTS>(registry, entity);

		EntityData* entityData = registry.try_get<EntityData>(entity);

		if (!entityData) {
			Logger::error("Failed to serialise entity {}.", static_cast<unsigned>(entity));
			return;
		}

		//set the parent of the root entity to entt::null
		if (checkParent) {
			json["EntityData"]["parent"] = static_cast<unsigned>(entt::null);
			checkParent = false;
		}

		jsonVec.push_back(json);

		for (entt::entity child : entityData->children) {
			serialisePrefabRecursive(registry, child, jsonVec, checkParent);
		}
	}

	PrefabEntityID deserialisePrefabRecursive(std::vector<Json> const& jsonVectorOfEntities, PrefabFileEntityID prefabFileEntityID, entt::registry& prefabRegistry, ResourceID prefabResourceId, std::unordered_map<PrefabFileEntityID, PrefabEntityID>& mapping, std::unordered_map<EntityGUID, PrefabEntityID>& entityGuidToPrefabId) {
		// In the vector of entities, let's find our json..
		for (auto const& jsonEntity : jsonVectorOfEntities) {
			PrefabFileEntityID fileEntityId = static_cast<entt::entity>(static_cast<unsigned>(jsonEntity["id"]));

			if (fileEntityId != prefabFileEntityID) {
				continue;
			}

			// We found our prefab entity data, let's deserialise it..
			PrefabEntityID prefabEntityId = prefabRegistry.create();
			deserialiseComponents<ALL_COMPONENTS>(prefabRegistry, prefabEntityId, jsonEntity);

			// We have successfully deserialised it. Let's deserialise it's children..
			// NOTE!! These ids in EntityData are in the domain of prefab file, let's deserialise it first..
			EntityData* entityData = prefabRegistry.try_get<EntityData>(prefabEntityId);

			if (!entityData) {
				Logger::warn("Missing / Failed to deserialise entity data for prefab file entity id {}", static_cast<unsigned>(fileEntityId));
				return entt::null;
			}

			// Let's first record the mapping between entity GUID and prefab entity id.. 
			entityGuidToPrefabId[entityData->entityGUID] = prefabEntityId;

			// Record mapping..
			mapping[fileEntityId] = prefabEntityId;

			// We store the mapped entity ids of the children for this prefab entity..
			std::vector<PrefabEntityID> prefabEntityChildrens;

			for (PrefabFileEntityID childPrefabFileEntityId : entityData->children) {
				// we recursively deserialise it's children..
				PrefabEntityID childPrefabEntityId = deserialisePrefabRecursive(jsonVectorOfEntities, childPrefabFileEntityId, prefabRegistry, prefabResourceId, mapping, entityGuidToPrefabId);

				if (childPrefabEntityId != entt::null) {
					prefabEntityChildrens.push_back(childPrefabEntityId);
				}

				// we update the fully deserialised child prefab entity with the parent's prefab entity id
				EntityData* childEntityData = prefabRegistry.try_get<EntityData>(childPrefabEntityId);

				if (!childEntityData) {
					Logger::warn("Missing / Failed to deserialise entity data for prefab file entity id {}", static_cast<unsigned>(childPrefabFileEntityId));
					return entt::null;
				}

				childEntityData->parent = prefabEntityId;
			}

			// Finally, we update prefab entity's prefab metadata..
			entityData->prefabID = { prefabResourceId };

			// As well as the mapped prefab children!
			entityData->children = std::move(prefabEntityChildrens);
			return prefabEntityId;
		}

		Logger::warn("Entity ID in prefab file does not correspond to any list of entity saved in the file!");
		return entt::null;
	}

	entt::id_type findLargestEntity(entt::registry& registry) {
		entt::entity highest = entt::null;

		for (entt::entity entity : registry.view<EntityData>()) {
			if (static_cast<entt::id_type>(entity) > static_cast<entt::id_type>(highest)) {
				highest = entity;
			}
		}

		return static_cast<entt::id_type>(highest);
	}
};