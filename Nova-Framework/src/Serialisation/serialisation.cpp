#include "serialisation.h"
#include "component.h"
#include <string>
#include <fstream>
#include <iomanip>

namespace Serialiser {
	void Serialiser::serialiseScene(entt::registry& registry, const char* fileName) {
		//try {
			Json js;
			std::vector<Json> jsonVec;

			for (auto&& [entity] : registry.view<entt::entity>().each()) {
				Json componentsJson = serialiseComponents<ALL_COMPONENTS>(registry, entity);
				jsonVec.push_back(componentsJson);
			}

			// save to output file
			js["entities"] = jsonVec;

			std::ofstream file(fileName);

			if (!file) {
				return;
			}

			file << std::setw(4) << js << std::endl;
		//}
		//catch (std::exception const& ex) {
		//	Logger::error("Failed to serialise scene. {}", ex.what());
		//}
	}

	void deserialiseScene(entt::registry& registry, const char* fileName) {
		try {
			std::ifstream file(fileName);

			if (!file.is_open())
				return;

			Json j;

			file >> j;

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
		Json j;

		std::ofstream file(fileName);

		if (!file.is_open())
			return;

		Json tempJ;

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

		Json j;
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

	//void deserialisePrefab(const char* fileName, entt::registry& registry, std::size_t id) {
	//std::vector<entt::entity> deserialisePrefab(const char* fileName, entt::registry& registry, std::size_t id, entt::registry& prefabRegistry) {
	void deserialisePrefab(const char* fileName, entt::registry& registry, std::size_t id, entt::registry& prefabRegistry, std::vector<entt::entity>& entityVec) {
	//entt::entity deserialisePrefab(ResourceFilePath fileName, entt::registry& registry, std::size_t id) {
		std::ifstream file(fileName);
		
		if (!file.is_open())
			return;

		Json j;
		file >> j;

		entt::entity highest = entt::null;
		entt::id_type highestID = 0;
		entt::entity rootEntity = entt::null;



		highestID = findLargestEntity(registry);

		//vector to store the child enities
		std::vector<entt::entity> childVec;

		//deserialise recursively starting from the child
		deserialisePrefabRecursive(j["Entities"], static_cast<int>(j["Entities"].size()) - 1, registry, highestID + 1, childVec, static_cast<int>(id), entityVec, prefabRegistry);
	}
	void serialisePrefab(entt::registry& registry, entt::entity entity, std::optional<std::ofstream> opt, std::size_t id) {
		std::ofstream& file = opt.value();

		if (!file.is_open())
			return;
		std::vector<Json> jsonVec;

		Json j;
		serialisePrefabRecursive(registry, entity, jsonVec, true, id);
		j["Entities"] = jsonVec;
		file << std::setw(4) << j << std::endl;

	}

	void serialisePrefabRecursive(entt::registry& registry, entt::entity entity, std::vector<Json>& jsonVec, bool checkParent, std::size_t id) {
		Json j = serialiseComponents<ALL_COMPONENTS>(registry, entity);

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
	void deserialisePrefabRecursive(std::vector<Json> jsonVec, int end, entt::registry& registry, entt::id_type highestID, std::vector<entt::entity>& childVec, std::size_t resourceid, std::vector<entt::entity>& entityVec, entt::registry& prefabRegistry) {
	//void deserialisePrefabRecursive(std::vector<Json> jsonVec, int end, entt::registry& registry, entt::id_type highestID, std::vector<entt::entity>& childVec, std::size_t resourceid, entt::entity& rootEntity, entt::registry& prefabRegistry) {
	//entt::entity deserialisePrefabRecursive(std::vector<Json> jsonVec, int end, entt::registry& registry, entt::id_type highestID, std::vector<entt::entity>& childVec, std::size_t resourceid) {
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
			//rootEntity = entity;
			/*rootEntity = entity;*/
		}
		else {
			childVec.push_back(entity);
		}
		entityVec.push_back(entity);

		deserialisePrefabRecursive(jsonVec, end-1, registry, highestID, childVec, resourceid, entityVec, prefabRegistry);
	}

	entt::id_type findLargestEntity(entt::registry& registry) {
		entt::id_type highestID{};
		entt::entity highest = entt::null;
		if (registry.view<EntityData>().size() == 0) {
			return highestID;
		}

		for (auto entity : registry.view<EntityData>()) {
			if (static_cast<entt::id_type>(entity) > highestID) {
				highest = entity;
				highestID = static_cast<entt::id_type>(highest);
			}
		}
		return highestID;
	}
};