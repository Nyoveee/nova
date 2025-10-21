#pragma once
#include "type_alias.h"
#include "customshader.h"
#include <map>
struct ShaderParserData {
	// Tags(Defaulted)
	CustomShader::BlendingConfig blendingConfig = CustomShader::BlendingConfig::AdditiveBlending;
	CustomShader::DepthTestingMethod depthTestingMethod = CustomShader::DepthTestingMethod::DepthTest;
	// Properties(name,type)
	std::unordered_map<std::string, std::string> uniforms;
	// Code
	std::string fShaderCode;
};
namespace ShaderParser
{
	bool Parse(AssetFilePath const& intermediaryAssetFilepath, ShaderParserData& shaderParserData);
};

