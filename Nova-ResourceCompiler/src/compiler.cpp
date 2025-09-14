#include "compiler.h"

#include "texture.h"
#include "model.h"
#include "cubemap.h"
#include "scriptAsset.h"
#include "audio.h"

#include <vector>

void Compiler::compileTexture(std::ofstream& resourceFile) {

}

void Compiler::compileModel(std::ofstream& resourceFile) {

}

void Compiler::compileCubeMap(std::ofstream& resourceFile) {

}

void Compiler::compileScriptAsset(std::ofstream& resourceFile) {

}

void Compiler::compileAudio(std::ofstream& resourceFile) {

}

void Compiler::compile(std::filesystem::path const& descriptorFilepath) {
	// Verify asset type.
	std::string resourceType = descriptorFilepath.parent_path().stem().string();

	if (resourceType == "Texture") {
		Compiler::compileAsset<Texture>(descriptorFilepath);
	}
	else if (resourceType == "Model") {
		Compiler::compileAsset<Model>(descriptorFilepath);
	}
	else if (resourceType == "CubeMap") {
		Compiler::compileAsset<CubeMap>(descriptorFilepath);
	}
	else if (resourceType == "ScriptAsset") {
		Compiler::compileAsset<ScriptAsset>(descriptorFilepath);
	}
	else if (resourceType == "Audio") {
		Compiler::compileAsset<Audio>(descriptorFilepath);
	}
	else {
		Logger::warn("Unable to determine asset type of descriptor {}", descriptorFilepath.string());
		return;
	}
}

void Compiler::test() {
	std::vector descriptorPaths {
		"C:\\Users\\Nyove\\Desktop\\nova\\Descriptors\\Texture\\Farm_Table_Low_Table_frame_mtl_BaseColor.desc",
		"C:\\Users\\Nyove\\Desktop\\nova\\Descriptors\\Model\\Farm_Table_Low.desc",
		"C:\\Users\\Nyove\\Desktop\\nova\\Descriptors\\Model\\box.desc",
		"C:\\Users\\Nyove\\Desktop\\nova\\Descriptors\\Audio\\SFX_AudioTest1.desc",
		"C:\\Users\\Nyove\\Desktop\\nova\\Descriptors\\CubeMap\\citrus_orchard_road_puresky_2k.desc",
		"C:\\Users\\Nyove\\Desktop\\nova\\Descriptors\\ScriptAsset\\TestScript.desc"
	};

	for (auto&& filepath : descriptorPaths) {
		compile(filepath);
	}
}