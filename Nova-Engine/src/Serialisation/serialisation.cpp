#include "serialisation.h"
#include <json/json.hpp>
#include "Component/component.h"
#include "ECS.h"

#include <fstream>
#include <iomanip>
constexpr const char* filepath = "test.json";

using json = nlohmann::json;

namespace Serialiser {
	void Serialiser::serialiseComponent(ECS& ecs) {
		entt::registry& registry = ecs.registry;

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
	}

	void deserialiseComponent(ECS& ecs) {
		json j;
		std::ifstream file(filepath);

		if (!file.is_open())
			return;

		file >> j;
		entt::registry& registry = ecs.registry;

		for (const auto& en : j["entities"])
		{
			auto entity = registry.create(en["id"]);

			Transform transform = {
				{en["Transform"]["position"]["x"], en["Transform"]["position"]["y"], en["Transform"]["position"]["z"]},
				{en["Transform"]["scale"]["x"], en["Transform"]["scale"]["y"], en["Transform"]["scale"]["z"]},
				{en["Transform"]["rotation"]["w"], en["Transform"]["rotation"]["x"], en["Transform"]["rotation"]["y"], en["Transform"]["rotation"]["z"]}
			};

			registry.emplace<Transform>(entity, std::move(transform));
			registry.emplace<EntityData>(entity, EntityData{ en["EntityData"]["name"], en["EntityData"]["parent"], en["EntityData"]["children"] });
		}
	}
};