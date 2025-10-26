#include "serialisation.h"
#include "ECS/component.h"
#include "ECS/ECS.h"
#include <string>
#include <fstream>
#include <iomanip>

namespace Serialiser {
	void Serialiser::serialiseScene(ECS& ecs, const char* fileName) {
		try {
			std::ofstream file(fileName);

			if (!file) {
				return;
			}

			entt::registry& registry = ecs.registry;

			json js;
			std::vector<json> jsonVec;

			for (auto&& [entity] : registry.view<entt::entity>().each()) {
				json componentsJson = serialiseComponents<ALL_COMPONENTS>(registry, entity);
				jsonVec.push_back(componentsJson);
			}

			// save to output file
			js["entities"] = jsonVec;
			file << std::setw(4) << js << std::endl;
		}
		catch (std::exception const& ex) {
			Logger::error("Failed to serialise scene. {}", ex.what());
		}
	}

	void deserialiseScene(ECS& ecs, const char* fileName) {
		try {
			std::ifstream file(fileName);

			if (!file.is_open())
				return;

			json j;

			file >> j;
			entt::registry& registry = ecs.registry;

			for (const auto& en : j["entities"]) {
				auto entity = registry.create(en["id"]);
				deserialiseComponents<ALL_COMPONENTS>(registry, entity, en);
			}
		}
		catch (std::exception const& ex) {
			Logger::error("Failed to deserialise scene. {}", ex.what());
		}
	}
	
	void serialiseGameConfig(const char* fileName, int gameWidth, int gameHeight) {
		json j;

		std::ofstream file(fileName);

		if (!file.is_open())
			return;

		json tempJ;

		tempJ["windowName"] = "Nova Game";
		tempJ["gameWidth"] = gameWidth;
		tempJ["gameHeight"] = gameHeight;

		j["Window"] = tempJ;
		tempJ.clear();

		file << std::setw(4) << j << std::endl;

	}

	void deserialiseGameConfig(const char* fileName, int& gameWidth, int& gameHeight, std::string& windowName) {
		std::ifstream file(fileName);

		if (!file.is_open())
			return;

		json j;
		file >> j;
		//j["Windows"]["windowName"];
		gameWidth = j["Window"]["gameWidth"];
		gameHeight = j["Window"]["gameHeight"];
		std::string str = j["Window"]["windowName"].dump();

		windowName = str.substr(str.find_first_not_of('"'), str.find_last_not_of('"'));
	}

	template <typename ...Windows>
	void serialiseEditorConfig(const char* fileName, bool console, bool debugUi, bool hierarchy, bool componentInspector) {
		std::ofstream file(fileName);

		if (!file.is_open())
			return;

		json j;
		json tempj;
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

		json j;
		file >> j;
	}

	void deserialisePrefab(const char* fileName, entt::registry& registry, std::size_t id) {
		std::ifstream file(fileName);
		
		if (!file.is_open())
			return;

		json j;
		file >> j;

		entt::entity highest = entt::null;
		entt::id_type highestID = 0;

		//find the highest entity
		for (auto&& [entity] : registry.view<entt::entity>().each()) {
			if (static_cast<entt::id_type>(entity) > highestID) {
				highest = entity;
				highestID = static_cast<entt::id_type>(highest);
			}
		}

		//vector to store the child enities
		std::vector<entt::entity> childVec;

		//deserialise recursively starting from the child
		deserialisePrefabRecursive(j["Entities"], j["Entities"].size() - 1, registry, highestID + 1, childVec, id);

	}
	void serialisePrefab(entt::registry& registry, entt::entity entity, std::optional<std::ofstream> opt, std::size_t id) {
		std::ofstream& file = opt.value();

		if (!file.is_open())
			return;
		std::vector<json> jsonVec;

		json j;
		serialisePrefabRecursive(registry, entity, jsonVec, true, id);
		j["Entities"] = jsonVec;
		file << std::setw(4) << j << std::endl;

	}

	void serialisePrefabRecursive(entt::registry& registry, entt::entity entity, std::vector<json>& jsonVec, bool checkParent, std::size_t id) {
		json j = serialiseComponents<ALL_COMPONENTS>(registry, entity);

		EntityData* entityData = registry.try_get<EntityData>(entity);
		entt::entity temp = entt::null;

		j["EntityData"]["prefabID"] = id;

		//set the parent of the root entity to entt::null
		if (checkParent) {
			j["EntityData"]["parent"] = temp;
			checkParent = false;
		}

		jsonVec.push_back(j);

		for (entt::entity child : entityData->children) {
			serialisePrefabRecursive(registry, child, jsonVec, checkParent, static_cast<std::size_t>(temp));
		}

	}
	void deserialisePrefabRecursive(std::vector<json> jsonVec, int end, entt::registry& registry, entt::id_type highestID, std::vector<entt::entity>& childVec, std::size_t resourceid) {
		if (end < 0) {
			return;
		}

		entt::id_type id = jsonVec[end]["id"] + highestID;
		auto entity = registry.create(static_cast<entt::entity>(id));
		deserialiseComponents<ALL_COMPONENTS>(registry, entity, jsonVec[end]);

		EntityData* entityData = registry.try_get<EntityData>(entity);

		// if its a parent, change the child vec and update the child's parent to the current entity
		if (entityData->children.size()) {
			entityData->children = childVec;
			for (entt::entity& child : entityData->children) {
				EntityData* childData = registry.try_get<EntityData>(child);
				childData->parent = entity;
			}
			childVec.clear();
		}
		if (end == 0) {
			entityData->prefabID = TypedResourceID<Prefab>{ resourceid };
		}
		else {
			childVec.push_back(entity);
		}

		deserialisePrefabRecursive(jsonVec, end-1, registry, highestID, childVec, resourceid);
	}
};