#include "AssetManager.h"

AssetManager::AssetManager() {
	addAsset<Texture>("Assets/Texture/PICHU.png");
	addAsset<ModelAsset>("Assets/Model/FarmTable_Textured.fbx");
	addAsset<ModelAsset>("Assets/Model/box.fbx");
}
