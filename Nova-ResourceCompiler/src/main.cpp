#include <iostream>
#include <fstream>
#include <filesystem>

#include "compiler.h"
#include "Logger.h"

#define TESTING false

// This program expects an argc count of 2, <executable> <path to descriptor file>
int main(int argc, const char* argv[]) {
	if (argc != 2) {
#if TESTING
		Compiler::test();
		return 0;
#else
		std::cerr << "Invalid amount of arguments!\n";
		return -1;
#endif
	}

	Compiler::compile(argv[1]);
	return 0;
}