#include "serialisation.h"
#include "Component/component.h"
#include "Component/ECS.h"

#include <fstream>
#include <iomanip>

constexpr const char* filepath = "test.json";

namespace Serialiser {
	void Serialiser::serialiseScene(ECS& ecs) {
		try {
			std::ofstream file(filepath);

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

	void deserialiseScene(ECS& ecs) {
		try {
			std::ifstream file(filepath);

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
};