#include "serialisation.h"
#include "component.h"
#include <string>
#include <fstream>
#include <iomanip>

namespace Serialiser {
	void Serialiser::serialiseScene(entt::registry& registry, std::vector<Layer> const& layers, const char* fileName) {

		Json js;
		std::vector<Json> jsonVec;

		for (auto&& [entity] : registry.view<entt::entity>().each()) {
			Json componentsJson = serialiseComponents<ALL_COMPONENTS>(registry, entity);
			jsonVec.push_back(componentsJson);
		}

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

		file << std::setw(4) << js << std::endl;
	}

	void deserialiseScene(entt::registry& registry, std::vector<Layer>& layers, const char* fileName) {
		try {
			std::ifstream file(fileName);

			if (!file.is_open())
				return;

			Json j;

			file >> j;

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
	
	void serialiseGameConfig(const char* fileName, GameConfig const& gameConfig) {
		try {
			// Load existing config to preserve other settings 
			std::ofstream outputFile(fileName);
			
			if (!outputFile) {
				return;
			}
			
			Json config;

			// Update window settings in config
			config["Window"]["gameWidth"] = gameConfig.gameWidth;
			config["Window"]["gameHeight"] = gameConfig.gameHeight;
			config["Window"]["windowName"] = gameConfig.gameName;
			config["Game"]["scene"] = static_cast<std::size_t>(gameConfig.sceneStartUp);

			outputFile << std::setw(4) << config << std::endl;
		}
		catch (const std::exception&) {}
	}

	GameConfig deserialiseGameConfig(const char* fileName) {
		GameConfig gameConfig;

		try {
			std::ifstream file(fileName);

			if (file.good()) {
				// std::cout << "Game config file found: " << configPath << std::endl;
				Json config = Json::parse(file);

				if (config.contains("Window")) {
					auto& windowConfig = config["Window"];

					if (windowConfig.contains("gameWidth")) {
						gameConfig.gameWidth = windowConfig["gameWidth"];
						//  std::cout << "Loaded game width: " << gameWidth << std::endl;
					}

					if (windowConfig.contains("gameHeight")) {
						gameConfig.gameHeight = windowConfig["gameHeight"];
						//  std::cout << "Loaded game height: " << gameHeight << std::endl;
					}

					if (windowConfig.contains("windowName")) {
						gameConfig.gameName = windowConfig["windowName"];
						//  std::cout << "Loaded window name: " << windowName << std::endl;
					}
				}

				if (config.contains("Game")) {
					auto& gameJson = config["Game"];

					if (gameJson.contains("scene")) {
						gameConfig.sceneStartUp = static_cast<std::size_t>(gameJson["scene"]);
					}
				}
			}
		}
		catch (const std::exception&) {}

		return gameConfig;
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

	entt::entity deserialisePrefab(const char* fileName, entt::registry& registry, [[maybe_unused]] std::size_t id, entt::registry& prefabRegistry) {
	//entt::entity deserialisePrefab(ResourceFilePath fileName, entt::registry& registry, std::size_t id) {
		std::ifstream file(fileName);
		
		if (!file.is_open())
			return entt::null;

		Json j;
		file >> j;

		//entt::entity rootEntity = entt::null;
		entt::entity rootEntity = j["RootEntity"];

		entt::id_type highestID = findLargestEntity(registry);

		std::unordered_map<entt::entity, entt::entity> map;

		//deserialisePrefabRecursive(j["Entities"],0, registry, highestID, static_cast<int>(id), rootEntity, prefabRegistry, entt::null, map);
		deserialisePrefabRecursive(j["Entities"], rootEntity, prefabRegistry, highestID, map);

		//return rootEntity;
		return j["RootEntity"];
	}
	void serialisePrefab(entt::registry& registry, entt::entity entity, std::optional<std::ofstream> opt, std::size_t id) {
		std::ofstream& file = opt.value();

		if (!file.is_open())
			return;
		std::vector<Json> jsonVec;

		Json j;
		j["RootEntity"] = entity;
		serialisePrefabRecursive(registry, entity, jsonVec, true, id);
		j["Entities"] = jsonVec;
		file << std::setw(4) << j << std::endl;

		Logger::debug("Prefab Serialized");

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

	template<typename ...Components>
	void deserialisePrefabRecursive(std::vector<Json> jsonVec, entt::entity& rootEntity, entt::registry& prefabRegistry, entt::id_type highestID, std::unordered_map<entt::entity, entt::entity>& map) {
	//void deserialisePrefabRecursive(std::vector<Json> jsonVec, int index, entt::registry& registry, entt::id_type highestID, std::size_t resourceid, entt::entity& rootEntity, entt::registry& prefabRegistry, entt::entity child, std::unordered_map<entt::entity, entt::entity>& map) {

		//find the entity in the json since it is unordered
		int index{};
		for (int i{}; i < jsonVec.size(); i++) {
			if (jsonVec[i]["id"] == static_cast<std::size_t>(rootEntity)) {
				index = i;
				break;
			}
		}
		

		//entt::id_type id = jsonVec[index]["id"] + highestID;
		//auto entity = prefabRegistry.create(static_cast<entt::entity>(id));
		auto entity = prefabRegistry.create(static_cast<entt::entity>(jsonVec[index]["id"]));
		deserialiseComponents<ALL_COMPONENTS>(prefabRegistry, entity, jsonVec[index]);

		EntityData* entityData = prefabRegistry.try_get<EntityData>(entity);

		if (entityData->children.size()) {
			for (entt::entity child : entityData->children) {
				deserialisePrefabRecursive<ALL_COMPONENTS>(jsonVec, child, prefabRegistry, highestID, map);
			}
		}

		//if (child != entt::null) {
		//	for (int i{}; i < jsonVec.size(); i++) {
		//		if (jsonVec[i]["id"] == static_cast<std::size_t>(child)) {
		//			index = i;
		//			break;
		//		}
		//	}
		//}

		//entt::id_type id = jsonVec[index]["id"] + highestID;
		//auto entity = registry.create(static_cast<entt::entity>(id));
		//deserialiseComponents<ALL_COMPONENTS>(registry, entity, jsonVec[index]);

		//map[child] = entity;

		//EntityData* entityData = registry.try_get<EntityData>(entity);

		//std::vector<entt::entity> entityVec;

		//if (entityData->children.size()) {
		//	for (entt::entity children : entityData->children) {
		//		deserialisePrefabRecursive(jsonVec, index, registry, highestID, resourceid, rootEntity, prefabRegistry, children, map);

		//		entityVec.push_back(map[children]);
		//	}
		//	entityData->children = entityVec;

		//	for (entt::entity en : entityVec) {
		//		EntityData* childData = registry.try_get<EntityData>(en);
		//		childData->parent = entity;
		//	}			
		//}

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