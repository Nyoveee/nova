#include "imgui.h"

template<ValidResource T>
void AssetViewerUI::displayAssetUI(ResourceID id, BasicAssetInfo& descriptor) {
	if constexpr (std::same_as<T, Texture>) {
		AssetInfo<Texture>& textureInfo = static_cast<AssetInfo<Texture>&>(descriptor);

		constexpr auto listOfEnumValues = magic_enum::enum_entries<AssetInfo<Texture>::Compression>();

		if (ImGui::BeginCombo("Compression", std::string{ magic_enum::enum_name<AssetInfo<Texture>::Compression>(textureInfo.compression) }.c_str())) {
			for (auto&& [enumValue, enumInString] : listOfEnumValues) {
				if (ImGui::Selectable(std::string{ enumInString }.c_str(), enumValue == textureInfo.compression)) {
					if (enumValue != textureInfo.compression) {
						textureInfo.compression = enumValue;

						// serialise immediately..
						assetManager.serialiseDescriptor<Texture>(selectedResourceId);

						// we make a copy of asset info, because the reference is getting invalidated..
						AssetInfo<Texture> textureInfoCopy = textureInfo;

						// we remove this old resource..
						resourceManager.removeResource(selectedResourceId);
						assetManager.removeResource(selectedResourceId);

						// recompile.., will add to resource manager if compilation is successful.
						assetManager.createResourceFile<Texture>(textureInfoCopy);
					}
				}
			}

			ImGui::EndCombo();
		}
	}
}
