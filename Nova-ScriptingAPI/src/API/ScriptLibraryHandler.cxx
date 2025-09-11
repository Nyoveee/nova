// Manages the global variables in the libraries shared by all scripts using callbacks and updates
#include "ScriptLibraryHandler.hxx"
#include "ScriptingAPI.hxx"
#include "inputManager.h"
#include "ScriptLibrary/Input.hxx"
using KeyValuePairFloat = System::Collections::Generic::KeyValuePair<bool,float>;
void ScriptLibraryHandler::init()
{
	Interface::engine->inputManager.subscribe<MousePosition>([](MousePosition currentMousePos) {
		Input::mousePosition_.x = static_cast<float>(currentMousePos.xPos);
		Input::mousePosition_.y = static_cast<float>(currentMousePos.yPos);
	});
	Interface::engine->inputManager.subscribe<AdjustCameraSpeed>([](AdjustCameraSpeed adjustCameraSpeed) {
		Input::scrollOffsetY_ = KeyValuePairFloat(true, static_cast<float>(adjustCameraSpeed.value));
	});
}

void ScriptLibraryHandler::update()
{
	// Make sure it resets after the next 1-2 frame
	// As this might trigger after the callback on the same frame instead of before
	if(Input::scrollOffsetY_.Key)
		Input::scrollOffsetY_ = KeyValuePairFloat(false,Input::scrollOffsetY_.Value);
	else
		Input::scrollOffsetY_ = KeyValuePairFloat(false, 0);
}