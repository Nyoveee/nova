#pragma once

#include "resource.h"
#include "systemResource.h"
#include <glm/glm.hpp>
#include "reflection.h"
#include "materialConfig.h"

class CustomShader;

// ================================================================================================================================
class Texture;

struct MaterialData {
	TypedResourceID<CustomShader>	selectedShader		{ DEFAULT_PBR_SHADER_ID };
	std::vector<UniformData>		uniformDatas		{ };
	BlendingConfig					blendingConfig		= BlendingConfig::Disabled;
	DepthTestingMethod				depthTestingMethod	= DepthTestingMethod::DepthTest;
	CullingConfig					cullingConfig		= CullingConfig::Enable;

	REFLECTABLE(
		selectedShader,
		uniformDatas,
		blendingConfig,
		depthTestingMethod,
		cullingConfig
	)
};
// ================================================================================================================================

class Material : public Resource {
public:
	FRAMEWORK_DLL_API Material(ResourceID id, ResourceFilePath resourceFilePath, MaterialData materialData);

public:
	MaterialData materialData;
};