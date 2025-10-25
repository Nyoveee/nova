#include "ShaderParser.h"

#include <fstream>
#include <sstream>
#include <regex>
#include <unordered_set>

#include "Logger.h"
#include "type_concepts.h"
#include "magic_enum.hpp"

/******************************************************************************
	Helpers
******************************************************************************/
template<IsEnum Enum> bool ParseEnum(std::regex regex, std::string const& string, Enum& output) {
	std::smatch matches;
	if (std::regex_search(string, matches, regex)) {
		constexpr auto enums = magic_enum::enum_entries<Enum>();
		for (auto const& [value,name] : enums) {
			std::string tagString{ matches[1] };
			if (tagString == name) {
				output = value;
				return true;
			}
		}
	}
	return false;
}
/******************************************************************************
	Sub Parsers
******************************************************************************/
bool ParseTags(std::string& data, CustomShader::ShaderParserData& shaderParserData)
{
	// Get full definition
	std::regex tagRegex{ R"(Tags\s*\{[\w\s:;]+\})" };
	auto taxRegexBegin{ std::sregex_iterator(std::begin(data),std::end(data),tagRegex) };
	ptrdiff_t count{ std::distance(taxRegexBegin,std::sregex_iterator()) };
	// Check Count
	if (count > 1) {
		Logger::error("Unable to Parse Shader, Multiple Tags Found");
		return false;
	}
	if (count == 0) return true;
	// Parse into shaderparserdata
	std::string result{ taxRegexBegin->str() };
	// Blending Config
	std::regex blendingConfigRegex{ R"(Blending\s*:\s*([\w]+)\w*;)" };
	if (!ParseEnum(blendingConfigRegex, result, shaderParserData.blendingConfig)) {
		Logger::error("Unable To Parse Shader, Unknown type associated with Blending Config");
		return false;
	}
	// Depth Testing Method
	std::regex depthTestingMethodRegex{ R"(DepthTestingMethod\s*:\s*([\w]+)\w*;)" };
	if (!ParseEnum(depthTestingMethodRegex, result, shaderParserData.depthTestingMethod)) {
		Logger::error("Unable To Parse Shader, Unknown type associated with Depth Testing Method");
		return false;
	}
	return true;
}
bool ParseUniforms(std::string& data, CustomShader::ShaderParserData& shaderParserData) {
	// Get full definition
	std::regex propertiesRegex{ R"(Properties[\s]*\{[\w\s;,]+\})" };
	auto propertiesRegexBegin{ std::sregex_iterator(std::begin(data),std::end(data),propertiesRegex) };
	ptrdiff_t count{ std::distance(propertiesRegexBegin,std::sregex_iterator()) };
	// Check Count
	if (count > 1) {
		Logger::error("Unable to Parse Shader, Multiple Properties Found");
		return false;
	}
	if (count == 0)
		return true;
	// Parse into shaderparserdata
	std::string result{ propertiesRegexBegin->str() };
	// Uniforms, not using type since sampler2D conflicts with int

	for (std::string const& propertyType : CustomShader::allValidShaderTypes) {
		std::string regexInput{ propertyType + R"(\s+([\w]+)\s*;)" };
		std::regex propertyRegex{ regexInput };
		std::string properties{ result };
		for (std::smatch matches; std::regex_search(properties, matches, propertyRegex);) {
			shaderParserData.uniforms[matches[1]] = propertyType;
			properties = matches.suffix();
		}
	}
	return true;
}
bool ParseFragmentShader(std::string& data, CustomShader::ShaderParserData& shaderParserData) {
	// Get full definition
	std::regex fragmentShaderRegex{ R"(Frag\s*\{[\*\-\+/\w\s=(.,);\[\]]+\})" };
	auto fragmentShaderRegexBegin{ std::sregex_iterator(std::begin(data),std::end(data),fragmentShaderRegex) };
	ptrdiff_t count{ std::distance(fragmentShaderRegexBegin,std::sregex_iterator()) };
	// Check Count
	if (count > 1) {
		Logger::error("Unable to Parse Shader, Multiple Fragments Found");
		return false;
	}
	if (count == 0)
		return true;
	// Parse into shaderparserdata
	std::string result{ fragmentShaderRegexBegin->str() };
	shaderParserData.fShaderCode = result.substr(result.find_first_of('{')+1, result.find_last_of('}') - result.find_first_of('{')-1);

	if (shaderParserData.fShaderCode.empty()) {
		return false;
	}

	return true;

}
/******************************************************************************
	Main Parsing function
******************************************************************************/ 
bool ShaderParser::Parse(AssetFilePath const& intermediaryAssetFilepath, CustomShader::ShaderParserData& shaderParserData)
{
	std::ifstream shaderFile{ intermediaryAssetFilepath };
	if(!shaderFile)
		return false;
	shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	std::string data;
	try {
		std::stringstream ss{};
		ss << shaderFile.rdbuf();
		data = ss.str();
	}
	catch (...) {
		return false;
	}
	// Parse Tags
	if (!ParseTags(data, shaderParserData))
		return false;
	if (!ParseUniforms(data, shaderParserData))
		return false;
	if (!ParseFragmentShader(data, shaderParserData))
		return false;
	
#if 1
	Logger::info("BlendingConfig = {}", magic_enum::enum_name(shaderParserData.blendingConfig));
	Logger::info("DepthTestingMethod = {}", magic_enum::enum_name(shaderParserData.depthTestingMethod));
	
	for (auto&& [identifier, type] : shaderParserData.uniforms) {
		Logger::info("Uniform = {}, Type = {}", identifier, type);
	}

	Logger::info("Fragment Shader Code\n{}", shaderParserData.fShaderCode);
#endif

	return true;
}
