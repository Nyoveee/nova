#include <iostream>
#include <fstream>
#include <filesystem>

#include "compiler.h"
#include "Logger.h"

#define DEBUGGING true

// This program expects an argc count of 2, <executable> <path to descriptor file>
int main([[maybe_unused]] int argc, [[maybe_unused]] const char* argv[]) {

	if (argc != 2) {
#if DEBUGGING
		std::filesystem::path testingPath = std::filesystem::current_path() / "Descriptors" / "Model" / "16153963675574681603.desc";
		return Compiler::compile(std::string{ testingPath.string().c_str() });
		//std::filesystem::path testingPath = std::filesystem::current_path() / "Descriptors" / "CustomShader" / "3036816346890944513.desc";
		//return Compiler::compile(std::string{ testingPath.string().c_str() });
#else
		std::cerr << "Invalid amount of arguments!\n";
		return -1;
#endif
	}

	return Compiler::compile(std::string{ argv[1] });
}