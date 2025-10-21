#include <iostream>
#include <fstream>
#include <filesystem>

#include "compiler.h"
#include "Logger.h"

#define DEBUGGING false

// This program expects an argc count of 2, <executable> <path to descriptor file>
int main([[maybe_unused]] int argc, [[maybe_unused]] const char* argv[]) {
#if DEBUGGING
	std::filesystem::path testingPath = std::filesystem::current_path() / "Descriptors" / "CustomShader" / "3036816346890944513.desc";
	return Compiler::compile(std::string{ testingPath.string().c_str() });
#else
	if (argc != 2) {
		std::cerr << "Invalid amount of arguments!\n";
		return -1;
	}

	return Compiler::compile(std::string{ argv[1] });
#endif
}