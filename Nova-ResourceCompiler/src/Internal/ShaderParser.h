#pragma once
#include "type_alias.h"
#include "customshader.h"

#include <map>

namespace ShaderParser
{
	bool Parse(AssetFilePath const& intermediaryAssetFilepath, CustomShader::ShaderParserData& shaderParserData);
};

