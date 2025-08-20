#include <glm/gtc/type_ptr.hpp>

#include "imgui.h"
#include "Component/component.h"

#include "componentInspector.h"

void displayMaterialUI(Material& material, ComponentInspector& componentInspector) {
	ImGui::BeginChild("Material UI", ImVec2{0, 400.f}, ImGuiChildFlags_Borders);

	// Handle albedo display.
	ImGui::SeparatorText("Albedo / Base color");

	std::visit([&](auto&& albedo) {
		using T = std::decay_t<decltype(albedo)>;

		if constexpr (std::same_as<T, AssetID>) {
			componentInspector.displayAssetDropDownList<Texture>(albedo, [&](AssetID selectedAssetId) {
				material.albedo = selectedAssetId;
			});
			
			if (ImGui::Button("Use color instead.")) {
				material.albedo = Color{ 0.1f, 0.1f, 0.1f };
				return;
			}
		}
		else /* its Color */ {
			glm::vec3 color = albedo;
			ImGui::ColorEdit3("color", glm::value_ptr(color));
			material.albedo = color;

			if (ImGui::Button("Use albedo map instead.")) {
				material.albedo = componentInspector.assetManager.getSomeAssetID<Texture>();
				return;
			}
		}

	}, material.albedo);
	
	ImGui::SeparatorText("Roughness, Metallic and Occulusion");
	
	std::visit([&](auto&& configuration) {
		using T = std::decay_t<decltype(configuration)>;

		if constexpr (std::same_as<T, AssetID>) {
			componentInspector.displayAssetDropDownList<Texture>(configuration, [&](AssetID selectedAssetId) {
				material.config = selectedAssetId;
			});

			if (ImGui::Button("Use constants instead.")) {
				material.config = Material::Config{ 0.5f, 0.f, 0.f };
				return;
			}
		}
		else /* its Config */ {
			ImGui::SliderFloat("Roughness", &configuration.roughness, 0.f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
			ImGui::SliderFloat("Metallic", &configuration.metallic, 0.f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
			ImGui::SliderFloat("Occulusion", &configuration.occulusion, 0.f, 1.f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

			if (ImGui::Button("Use packed map instead.")) {
				material.config = componentInspector.assetManager.getSomeAssetID<Texture>();
				return;
			}
		}

	}, material.config);

	ImGui::SeparatorText("Normal");

	if (!material.normalMap) {
		if (ImGui::Button("Use a normal map.")) {
			material.normalMap = componentInspector.assetManager.getSomeAssetID<Texture>();
		}
	}
	else {
		componentInspector.displayAssetDropDownList<Texture>(material.normalMap.value(), [&](AssetID selectedAssetId) {
			material.normalMap = selectedAssetId;
		});

		if (ImGui::Button("Remove normal map.")) {
			material.normalMap = std::nullopt;
		}
	}

	ImGui::EndChild();
}