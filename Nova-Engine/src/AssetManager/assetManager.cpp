#include "AssetManager.h"

AssetManager::AssetManager() {
	addAsset<Texture>("Assets/Texture/PICHU.png");
	addAsset<Model>("Assets/Model/FarmTable_Textured.fbx");
	addAsset<Model>("Assets/Model/box.fbx");
	addAsset<Texture>("Assets/Texture/Table_frame_mtl_Base_color.png");
	addAsset<Texture>("Assets/Texture/Table_top_mtl_Base_color.png");
}
