#include "Engine/engine.h"
#include "editor.h"

#include "loadScene.h"

LoadScene::LoadScene(Editor& editor) :
	ecs{ editor.engine.ecs },
	editor{ editor }
{
}