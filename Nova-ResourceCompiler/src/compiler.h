#pragma once

#include <optional>
#include <fstream>

#include "asset.h"

// using class instead of namespace because i want the ability to private certain functions.
class Compiler {
public:
	static void compile(std::filesystem::path const& descriptorFilepath);

	// debug function to test.
	static void test();

private:
	template <ValidAsset T>
	static void compileAsset(std::filesystem::path const& descriptorFilepath);

	static void compileTexture(std::ofstream& resourceFile);
	static void compileModel(std::ofstream& resourceFile);
	static void compileCubeMap(std::ofstream& resourceFile);
	static void compileScriptAsset(std::ofstream& resourceFile);
	static void compileAudio(std::ofstream& resourceFile);
};

#include "compiler.ipp"