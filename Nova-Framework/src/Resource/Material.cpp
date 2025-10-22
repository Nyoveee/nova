#include "Material.h"

Material::Material(ResourceID id, ResourceFilePath resourceFilePath, MaterialData materialData)
	: Resource		{ id, resourceFilePath }
	, materialData	{ std::move(materialData) } 
{}

Material::~Material()
{}