#include <iostream>
#include <fstream>
#include <filesystem>

#include "compiler.h"
#include "Logger.h"

#define DEBUGGING true

// This program expects an argc count of 2, <executable> <path to descriptor file>
int main(int argc, const char* argv[]) {
#if DEBUGGING
	const char* test = R"(C:\Users\Nyove\Desktop\nova\Descriptors\Texture\8166196387750379525.desc)";
	return Compiler::compile(std::string{ test });
#else
	if (argc != 2) {
		std::cerr << "Invalid amount of arguments!\n";
		return -1;
	}

	return Compiler::compile(std::string{ argv[1] });
#endif
}