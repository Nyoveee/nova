#include "Material.h"

Material::Material(ResourceID id, ResourceFilePath resourceFilePath, MaterialData const& materialData)
	: Resource{id, resourceFilePath}
	, materialData{ materialData } {
}

Material::~Material()
{
}