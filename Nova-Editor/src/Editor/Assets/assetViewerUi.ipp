#include "imgui.h"
#include "assetViewerUi.h"

template<ValidResource T>
void AssetViewerUI::displayAssetUI(BasicAssetInfo& descriptor) {
	AssetInfo<T>& typedDescriptor = static_cast<AssetInfo<T>&>(descriptor);

	if constexpr (std::same_as<T, Texture>) {
		displayTextureInfo(typedDescriptor);
	}
	else if constexpr (std::same_as<T, Model>) {
		displayModelInfo(typedDescriptor);
	}
	else if constexpr (std::same_as <T, Material>) {
		displayMaterialInfo(typedDescriptor);
	}
	else if constexpr (std::same_as <T, CustomShader>) {
		displayShaderInfo(typedDescriptor);
	}
	else if constexpr (std::same_as <T, Font>) {
		displayFontInfo(typedDescriptor);
	}
	else if constexpr (std::same_as<T, Prefab>) {
		displayPrefabInfo(typedDescriptor);
	}
	else if constexpr (std::same_as<T, ScriptAsset>) {
		displayScriptInfo(typedDescriptor);
	}
}


template <typename T>
void AssetViewerUI::recompileResourceWithUpdatedDescriptor(AssetInfo<T> const& p_assetInfo) {
	recompileAssetWithDescriptor = [&, assetInfo = p_assetInfo, id = selectedResourceId]() {
		// serialise immediately..
		assetManager.serialiseDescriptor(id);

		// we make a copy of asset info, because the reference is getting invalidated..
		AssetInfo<T> assetInfoCopy = assetInfo;

		// we remove this old resource..
		resourceManager.removeResource(id);
		assetManager.removeResource(id);

		// recompile.., will add to resource manager if compilation is successful.
		assetManager.createResourceFile<T>(assetInfoCopy);
	};
}