#pragma once
#include <string>
#include <msclr/marshal_cppstd.h>
#include <functional>
#include "ScriptLibrary/ManagedEnums.hxx"
// For additional conversions
// Managed Component

template<typename ManagedType>
auto Convert(ManagedType^ managedType) {
	return !managedType ? nullptr : managedType->nativeComponent(); // Only components have this function call
}

// Strings
inline std::string Convert(System::String^ str) {
	return msclr::interop::marshal_as<std::string>(str);
}

// Event Callback
public delegate void EventCallback();
template<typename T> 
std::function<void(T)> Convert(EventCallback^ callback, Key key) {
	gcroot<EventCallback^> callbackWrapper; // For Wrapping the callback function in a native class gcroot since lambda can't use the callbacks directly
	int keyValue = safe_cast<int>(key);
	// Press
	callbackWrapper = callback;
	auto pressCallbackFunction = [callbackWrapper, keyValue](T inputEvent) {
		if (callbackWrapper && static_cast<int>(inputEvent) == keyValue)
			callbackWrapper.operator->()(); // Bit of a weird syntax, first get the object of type InputCallback^, then call the function 
	};
	return pressCallbackFunction;
}
