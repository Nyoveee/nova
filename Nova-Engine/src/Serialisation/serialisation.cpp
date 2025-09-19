#include "serialisation.h"
#include "Component/component.h"
#include "Component/ECS.h"
#include <string>

#include <fstream>
#include <iomanip>
//constexpr const char* filetest2path = "test2.json";
constexpr const char* filepath = "test.json";

namespace Serialiser {
	void Serialiser::serialiseScene(ECS& ecs, const char* fileName) {
		//std::ofstream file(filetest2path);
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

#if 0
		// add your implementation here.
		
		json tempJson;
		json transformJson;
		json positionJson;
		json scaleJson;
		json rotationJson;
		json j;
		json js;

		std::vector<json> jsonVec;

		std::ofstream file(filepath);

		for (auto&& [entity, transform, entityData] : registry.view<Transform, EntityData>().each()) {
			
			j["id"] = entity;

			tempJson["name"] = entityData.name;
			tempJson["parent"] = entityData.parent;
			tempJson["children"] = entityData.children;

			j["EntityData"] = tempJson;
			tempJson.clear();

			tempJson["x"] = transform.position.x;
			tempJson["y"] = transform.position.y;
			tempJson["z"] = transform.position.z;

			positionJson["position"] = tempJson;
			tempJson.clear();

			tempJson["x"] = transform.scale.x;
			tempJson["y"] = transform.scale.y;
			tempJson["z"] = transform.scale.z;
			scaleJson["scale"] = tempJson;
			tempJson.clear();

			tempJson["w"] = transform.rotation.w;
			tempJson["x"] = transform.rotation.x;
			tempJson["y"] = transform.rotation.y;
			tempJson["z"] = transform.rotation.z;
			rotationJson["rotation"] = tempJson;
			tempJson.clear();

			transformJson.update(positionJson);
			transformJson.update(scaleJson);
			transformJson.update(rotationJson);
			j["Transform"] = transformJson;

			jsonVec.push_back(j);
		}

		js["entities"] = jsonVec;
		file << std::setw(4) << js << std::endl;
#endif
	}

	void deserialiseScene(ECS& ecs, const char* fileName) {
		(void)ecs;

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
};