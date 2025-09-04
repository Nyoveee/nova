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
		json jTemp;
		std::ofstream file(filepath);

		std::vector<json> jsonVec;

		for (auto&& [entity, transform, entityData] : registry.view<Transform, EntityData>().each())
		{
			jTemp["name"] = entityData.name;
			jTemp["position"] = { transform.position.x, transform.position.y, transform.position.z };
			jTemp["scale"] = { transform.scale.x, transform.scale.y, transform.scale.z };
			jTemp["rotation"] = { transform.rotation.w, transform.rotation.x, transform.rotation.y, transform.rotation.z };

			jsonVec.push_back(jTemp);
			jTemp.clear();
		}
		json j;

		j["test"] = jsonVec;
		file << std::setw(4) << j << std::endl;
	}
};