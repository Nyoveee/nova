#include "systemResource.h"
#include "assetIO.h"
#include "Material.h"

// WE DEFINE ALL SYSTEM RESOURCES HERE!

#define SystemFilePath(AssetType, filepath) \
	std::filesystem::current_path() / "System" / #AssetType / filepath

// =============================== Models =============================== 
const std::unordered_map<ResourceID, ResourceFilePath> AssetIO::systemModelResources {
	{	CUBE_MODEL_ID,					SystemFilePath(Model, "Cube")		},
	{	SPHERE_MODEL_ID,				SystemFilePath(Model, "Sphere")		},
	{	CAPSULE_MODEL_ID,				SystemFilePath(Model, "Capsule")	}
};

// =============================== Shader =============================== 
const std::unordered_map<ResourceID, ResourceFilePath> AssetIO::systemShaderResources{
	{	DEFAULT_PBR_SHADER_ID,			SystemFilePath(CustomShader, "[Default] PBR Shader")	},
	{	DEFAULT_COLOR_SHADER_ID,		SystemFilePath(CustomShader, "[Default] Color Shader")	}
};

// =============================== Material =============================== 
const std::unordered_map<ResourceID, ResourceFilePath> AssetIO::systemMaterialResources{
	{	DEFAULT_PBR_MATERIAL_ID,		SystemFilePath(Material, "[Default] PBR Material")		},
	{	DEFAULT_COLOR_MATERIAL_ID,		SystemFilePath(Material, "[Default] Color Material")	}
};

// =============================== Texture =============================== 
const std::unordered_map<ResourceID, ResourceFilePath> AssetIO::systemTextureResources{
	{	NONE_TEXTURE_ID,				SystemFilePath(Texture, "None")			},
	{	INVALID_TEXTURE_ID,				SystemFilePath(Texture, "MissingTex")	}
};