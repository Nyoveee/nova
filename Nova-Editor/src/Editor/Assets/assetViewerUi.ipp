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
		displayMaterialInfo();
	}
}
