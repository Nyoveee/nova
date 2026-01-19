#include "imgui.h"
#include "component.h"
#include "PropertyDisplay/displayProperties.h"
#include "API/assetSerializer.h"

void displayReflectionProbeComponent(Editor& editor, ReflectionProbe& reflectionProbe, Transform const& transform) {
	DisplayProperty<glm::vec3>(editor, "Box extents", reflectionProbe.boxExtents);
	DisplayProperty<glm::vec3>(editor, "Probe position offset", reflectionProbe.centerOffset);

	// box extents will always be +ve.
	reflectionProbe.boxExtents = glm::abs(reflectionProbe.boxExtents);
	
	// Let's keep the offset within the boundary confinments shall we?
	reflectionProbe.centerOffset = glm::max(glm::min(reflectionProbe.centerOffset, reflectionProbe.boxExtents), -reflectionProbe.boxExtents);

	// We need to find the appropriate longest distance, so that we can clamp our capture radius..
	// Let's calculate the hypotenuse from probePosition to min and max. Let's keep it squared.
	// https://community.khronos.org/t/glm-vector-distancesquare/65282
	float distanceSquared = glm::dot(reflectionProbe.boxExtents, reflectionProbe.boxExtents);

	DisplayProperty<float>(editor, "Capture radius", reflectionProbe.captureRadius);

	if ((reflectionProbe.captureRadius * reflectionProbe.captureRadius) < distanceSquared) {
		reflectionProbe.captureRadius = std::sqrtf(distanceSquared);
	}

	ImGui::SeparatorText("Maps used");

	DisplayProperty<TypedResourceID<CubeMap>>(editor, "Irradiance map", reflectionProbe.irradianceMap);
	DisplayProperty<TypedResourceID<CubeMap>>(editor, "Prefiltered environment map", reflectionProbe.prefilteredEnvironmentMap);

	if (ImGui::	Button("Bake")) {
		AssetSerializer::serialiseCubeMap(editor.engine.renderer.bakeSpecularIrradianceMap(reflectionProbe, transform.position));
	}
}	