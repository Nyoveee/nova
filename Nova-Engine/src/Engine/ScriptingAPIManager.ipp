template<typename Func>
Func ScriptingAPIManager::getCoreClrFuncPtr(const std::string& functionName) {
	// Get function from dll
	Func fptr = reinterpret_cast<Func>(GetProcAddress(coreClr, functionName.c_str()));
	if (!fptr)
		throw std::runtime_error("Unable to get pointer to function.");
	return fptr;
}

template<typename Func>
Func ScriptingAPIManager::GetFunctionPtr(std::string typeName, std::string functionName) {
	static const std::string assemblyName{ "Nova-ScriptingAPI" };

	Func managedDelegate{ nullptr };
	int result = createManagedDelegate(
		hostHandle,
		domainID,
		assemblyName.data(),
		typeName.data(),
		functionName.data(),
		reinterpret_cast<void**>(&managedDelegate)
	);
	if (result != S_OK) {
		std::ostringstream errorDetails;
		errorDetails << "(0x";
		errorDetails << std::hex << result;
		errorDetails << std::string{ ") Unable to Get Function " + assemblyName + '|' + typeName + '|' + functionName }.c_str();
		throw std::runtime_error(errorDetails.str());
	}
	return managedDelegate;
}