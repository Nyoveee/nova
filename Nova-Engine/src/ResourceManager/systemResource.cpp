#include "resourceManager.h"

// WE DEFINE ALL SYSTEM RESOURCES HERE!

#define SystemFilePath(AssetType, filepath) \
	std::filesystem::current_path() / "System" / #AssetType / filepath

// =============================== Models =============================== 
constexpr ResourceID CUBE_MODEL_ID		= 1;
constexpr ResourceID SPHERE_MODEL_ID	= 2;

const std::unordered_map<ResourceID, ResourceFilePath> ResourceManager::systemModelResources {
	// =============================== Models =============================== 
	{	CUBE_MODEL_ID,		SystemFilePath(Model, "Cube")		},
	{	SPHERE_MODEL_ID,	SystemFilePath(Model, "Sphere")		}
};