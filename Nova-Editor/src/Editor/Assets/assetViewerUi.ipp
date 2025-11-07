#include "imgui.h"

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
}

template <typename T>
void AssetViewerUI::recompileResourceWithUpdatedDescriptor(AssetInfo<T> const& assetInfo) {
	// serialise immediately..
	assetManager.serialiseDescriptor<T>(selectedResourceId);

	// we make a copy of asset info, because the reference is getting invalidated..
	AssetInfo<T> assetInfoCopy = assetInfo;

	// we remove this old resource..
	resourceManager.removeResource(selectedResourceId);
	assetManager.removeResource(selectedResourceId);

	// recompile.., will add to resource manager if compilation is successful.
	assetManager.createResourceFile<T>(assetInfoCopy);
}