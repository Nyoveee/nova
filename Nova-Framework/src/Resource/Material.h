#pragma once

#include "resource.h"
#include <glm/glm.hpp>
#include "reflection.h"

class CustomShader;

class Material : public Resource {
public:
	FRAMEWORK_DLL_API Material(ResourceID id, ResourceFilePath resourceFilePath, MaterialData materialData);
	FRAMEWORK_DLL_API ~Material();

	FRAMEWORK_DLL_API Material(Material const& other) = delete;
	FRAMEWORK_DLL_API Material(Material&& other) = delete;
	FRAMEWORK_DLL_API Material& operator=(Material const& other) = delete;
	FRAMEWORK_DLL_API Material& operator=(Material&& other) = delete;

public:
	MaterialData materialData;
};