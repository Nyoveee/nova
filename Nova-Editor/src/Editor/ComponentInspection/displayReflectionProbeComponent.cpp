#include "imgui.h"
#include "component.h"
#include "PropertyDisplay/displayProperties.h"
#include "API/assetSerializer.h"

void displayReflectionProbeComponent(Editor& editor, ReflectionProbe& reflectionProbe, Transform const& transform) {
	DisplayProperty<glm::vec3>(editor, "Box extents", reflectionProbe.boxExtents);
	DisplayProperty<glm::vec3>(editor, "Probe position offset", reflectionProbe.centerOffset);
	
	DisplayProperty<float>(editor, "Intensity", reflectionProbe.intensity);
	ImGui::SliderFloat("Blending Fall Off", &reflectionProbe.fallOff, 0.f, 1.f);

	// box extents will always be +ve.
	reflectionProbe.boxExtents = glm::abs(reflectionProbe.boxExtents);
	
	// Let's keep the offset within the boundary confinments shall we?
	reflectionProbe.centerOffset = glm::max(glm::min(reflectionProbe.centerOffset, reflectionProbe.boxExtents), -reflectionProbe.boxExtents);

	// We need to find the appropriate longest distance, so that we can clamp our capture radius..
	// Let's calculate the hypotenuse from probePosition to min and max. Let's keep it squared.
	// https://community.khronos.org/t/glm-vector-distancesquare/65282
	float distanceSquared = glm::dot(reflectionProbe.boxExtents, reflectionProbe.boxExtents);

	ImGui::SeparatorText("Maps used");

	editor.displayAssetDropDownList<CubeMap>(reflectionProbe.irradianceMap, "Irradiance map", [&](ResourceID id) {
		reflectionProbe.irradianceMap = { id };
	});

	editor.displayAssetDropDownList<CubeMap>(reflectionProbe.prefilteredEnvironmentMap, "Prefiltered environment map", [&](ResourceID id) {
		reflectionProbe.prefilteredEnvironmentMap = { id };

		auto&& [cubeMap, _] = editor.resourceManager.getResource<CubeMap>(reflectionProbe.prefilteredEnvironmentMap);

		if (cubeMap) {
			editor.engine.renderer.loadReflectionProbe(reflectionProbe, *cubeMap);
		}
	});

	ImGui::SeparatorText("Capture settings");

	DisplayProperty<float>(editor, "Capture radius", reflectionProbe.captureRadius);

	if ((reflectionProbe.captureRadius * reflectionProbe.captureRadius) < distanceSquared) {
		reflectionProbe.captureRadius = std::sqrtf(distanceSquared);
	}

	ImGui::Checkbox("Capture environment light?", &reflectionProbe.toCaptureEnvironmentLight);

	if (ImGui::	Button("Bake")) {
		AssetSerializer::serialiseCubeMap(editor.engine.renderer.bakePrefilteredEnvironmentMap(reflectionProbe, transform.position, reflectionProbe.toCaptureEnvironmentLight));
	}
}	