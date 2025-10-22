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
};