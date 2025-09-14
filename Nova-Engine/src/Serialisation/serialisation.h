#pragma once

#include <fstream>
#include <entt/entt.hpp>
#include <json/json.hpp>

class ECS;

using json = nlohmann::json;

namespace Serialiser {
	void serialiseScene(ECS& ecs);
	void deserialiseScene(ECS& ecs);

	template <typename ...Components>
	json serialiseComponents(entt::registry& registry, entt::entity entity);

	template <typename ...Components>
	void deserialiseComponents(entt::registry& registry, entt::entity entity, std::ifstream& inputFile, json en);
	//void deserialiseComponents(entt::registry& registry, entt::entity entity, std::ifstream& inputFile);

	template <typename T>
	json serialiseComponent(T& component);

	template <typename T>
	//void deserialiseComponent(std::ifstream& inputFile, json jsonComponent);
	void deserialiseComponent(json jsonComponent); // this was the orginal starting point 
	

};

#include "serialisation.ipp"