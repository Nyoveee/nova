#pragma once

#include <fstream>

class ECS;

namespace SceneManager {

	class SceneManager {
	public:
		SceneManager(ECS& ecs, const char* fileName);
		void LoadScene(ECS& ecs, const char* inputFile, const char* ouputFile);
	};
	
};