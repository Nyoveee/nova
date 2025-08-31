#include "Input.hxx"
#include "API/ScriptingAPI.hxx"
#include "inputManager.h"
#include <vcclr.h>
void Input::MapKey(Key key, InputCallback^ pressCallback, InputCallback^ releaseCallback)
{
	gcroot<InputCallback^> callbackWrapper; // For Wrapping the callback function in a native class gcroot since lambda can't use the callbacks directly
	int keyValue = safe_cast<int>(key);
	// Press
	callbackWrapper = pressCallback; 
	auto pressCallbackFunction = [callbackWrapper, keyValue](ScriptingInputEvents inputEvent){
		if(callbackWrapper && static_cast<int>(inputEvent) == keyValue)
			callbackWrapper.operator->()(); // Bit of a weird syntax, first get the object of type InputCallback^, then call the function 
	}; 
	// Release
	callbackWrapper = releaseCallback; 
	auto releaseCallbackFunction = [callbackWrapper, keyValue](ScriptingInputEvents inputEvent) {
		if (callbackWrapper && static_cast<int>(inputEvent) == keyValue)
			callbackWrapper.operator->()(); // Bit of a weird syntax, first get the object of type InputCallback^, then call the function 
	};
	Interface::engine->inputManager.subscribe<ScriptingInputEvents>(pressCallbackFunction, releaseCallbackFunction);
}
