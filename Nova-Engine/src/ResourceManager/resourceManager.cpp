#include "resourceManager.h"
#include "Logger.h"

ResourceManager::ResourceManager() :
	resourceDirectory{ std::filesystem::current_path() /= "Resources" }
{
	// ========================================
	// 1. Check if the resource directory exist.
	// ========================================
	if (!std::filesystem::exists(resourceDirectory)) {
		// create the folder if it doesn't exist.
		try {
			std::filesystem::create_directory(resourceDirectory);
		}
		catch (const std::filesystem::filesystem_error& ex) {
			Logger::error("Filesystem error: {}", ex.what());
		}
	}

	// ========================================
	// 2. Load all the assets.
	// ========================================
	try {
		for (const auto& entry : std::filesystem::recursive_directory_iterator{ resourceDirectory }) {
			std::filesystem::path currentPath = entry.path();

			if (entry.is_regular_file()) {
				// we add asset
			}
		}
	}
	catch (const std::filesystem::filesystem_error& ex) {
		Logger::error("Filesystem error: {}", ex.what());
	}
}
