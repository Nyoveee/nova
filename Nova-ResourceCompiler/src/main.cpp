#include <iostream>
#include <fstream>
#include <filesystem>

#include "compiler.h"
#include "Logger.h"

#define DEBUGGING true
#define RECOMPILE_ALL_SYSTEM_RESOURCES false

// This program expects an argc count of 2, <executable> <path to descriptor file>
int main([[maybe_unused]] int argc, [[maybe_unused]] const char* argv[]) {

	if (argc != 2) {
#if DEBUGGING
#if RECOMPILE_ALL_SYSTEM_RESOURCES 
		Compiler::recompileAllSystemAssets(); 
#endif
		//std::filesystem::path testingPath = std::filesystem::current_path() / "Descriptors" / "CustomShader" / "8851781646620934145.desc";
		//return Compiler::compile(std::string{ testingPath.string().c_str() });

		return 0;
#else
		std::cerr << "Invalid amount of arguments!\n";
		return -1;
#endif
	}

	return Compiler::compile(std::string{ argv[1] });
}