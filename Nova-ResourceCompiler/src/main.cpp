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
		std::filesystem::path testingPath1 = std::filesystem::current_path() / "Descriptors" / "Model" / "9752316694208315393.desc";
		//std::filesystem::path testingPath2 = std::filesystem::current_path() / "Descriptors" / "Model" / "3554510369322696707.desc";
		//std::filesystem::path testingPath3 = std::filesystem::current_path() / "Descriptors" / "Model" / "9752316694208315393.desc";

		return Compiler::compile(std::string{ testingPath1.string().c_str() });
		//return Compiler::compile(std::string{ testingPath2.string().c_str() });
#else
		std::cerr << "Invalid amount of arguments!\n";
		return -1;
#endif
	}

	return Compiler::compile(std::string{ argv[1] });
}