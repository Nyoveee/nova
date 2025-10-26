#include <glm/gtc/type_ptr.hpp>

#include "imgui.h"
#include "component.h"

#include "Editor/ComponentInspection/PropertyDisplay/displayProperties.h"

#include "magic_enum.hpp"

#include "ResourceManager/resourceManager.h"
#include "Editor/editor.h"

void displayMaterialUI(Material& material, ComponentInspector& componentInspector) {
	ImGui::BeginChild("Material UI", ImVec2{0, 400.f}, ImGuiChildFlags_Borders);

	// ============================================
	// Handle rendering pipeline dropdown box.
	// ============================================

	ImGui::SeparatorText("Rendering pipeline.");

	const char* pipelineName;

	switch (material.renderingPipeline)
	{
	case Material::Pipeline::PBR:
		pipelineName = "Physical Based Rendering.";
		break;
	case Material::Pipeline::BlinnPhong:
		pipelineName = "Blinn-Phong Shading.";
		break;
	case Material::Pipeline::Color:
		pipelineName = "Plain old color.";
		break;
	default:
		pipelineName = "easter egg!";
	}

	if (ImGui::BeginCombo("##pipeline", pipelineName)) {
		if (ImGui::Selectable("Physical Based Rendering.", material.renderingPipeline == Material::Pipeline::PBR)) {
			material.renderingPipeline = Material::Pipeline::PBR;
		}

		if (ImGui::Selectable("Blinn-Phong Shading.", material.renderingPipeline == Material::Pipeline::BlinnPhong)) {
			material.renderingPipeline = Material::Pipeline::BlinnPhong;
		}

		if (ImGui::Selectable("Plain old color.", material.renderingPipeline == Material::Pipeline::Color)) {
			material.renderingPipeline = Material::Pipeline::Color;
		}

		ImGui::EndCombo();
	}

	// ============================================
	// Handle albedo property display.
	// ============================================

	ImGui::SeparatorText(material.renderingPipeline == Material::Pipeline::Color ? "Color" : "Albedo / Base color");

	std::visit([&](auto&& albedo) {
		using T = std::decay_t<decltype(albedo)>;

		if constexpr (std::same_as<T, ResourceID>) {
			componentInspector.editor.displayAssetDropDownList<Texture>(albedo, "Albedo Map", [&](ResourceID selectedAssetId) {
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

			if (ImGui::Button(material.renderingPipeline == Material::Pipeline::Color ? "Use texture instead." : "Use albedo map instead.")) {
				material.albedo = componentInspector.resourceManager.getSomeResourceID<Texture>();
				return;
			}
		}

	}, material.albedo);
	
	// ============================================
	// Handle Roughness, Metallic and Occulusion property display.
	// ============================================

	ImGui::SeparatorText("Roughness, Metallic and Occulusion");

	if (material.renderingPipeline != Material::Pipeline::PBR) {
		ImGui::BeginDisabled();
		ImGui::Text("Only PBR pipeline uses these properties.");
	}

	std::visit([&](auto&& configuration) {
		using T = std::decay_t<decltype(configuration)>;

		if constexpr (std::same_as<T, ResourceID>) {
			componentInspector.editor.displayAssetDropDownList<Texture>(configuration, "Packed Texture Map", [&](ResourceID selectedAssetId) {
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
				material.config = componentInspector.resourceManager.getSomeResourceID<Texture>();
				return;
			}
		}

	}, material.config);

	if (material.renderingPipeline != Material::Pipeline::PBR) {
		ImGui::EndDisabled();
	}

	// ============================================
	// Handle normal property display.
	// ============================================
	ImGui::SeparatorText("Normal");

	if (material.renderingPipeline == Material::Pipeline::Color) {
		ImGui::BeginDisabled();
		ImGui::Text("The color pipeline does not use a normal map.");
	}

	if (!material.normalMap) {
		if (ImGui::Button("Use a normal map.")) {
			material.normalMap = componentInspector.resourceManager.getSomeResourceID<Texture>();
		}
	}
	else {
		componentInspector.editor.displayAssetDropDownList<Texture>(material.normalMap.value(), "Normal Map", [&](ResourceID selectedResourceId) {
			material.normalMap = selectedResourceId;
		});

		if (ImGui::Button("Remove normal map.")) {
			material.normalMap = std::nullopt;
		}
	}

	if (!material.emissiveMap) {
		if (ImGui::Button("Use a emissive map.")) {
			material.emissiveMap = componentInspector.resourceManager.getSomeResourceID<Texture>();
		}
	}
	else {
		componentInspector.editor.displayAssetDropDownList<Texture>(material.emissiveMap.value(), "Emissive Map", [&](ResourceID selectedResourceId) {
			material.emissiveMap = selectedResourceId;
			});

		if (ImGui::Button("Remove emissive map.")) {
			material.emissiveMap = std::nullopt;
		}
	}

	if (material.renderingPipeline == Material::Pipeline::Color) {
		ImGui::EndDisabled();
	}

	ImGui::EndChild();
}