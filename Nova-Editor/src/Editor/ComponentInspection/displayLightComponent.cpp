#include "imgui.h"
#include "component.h"
#include "PropertyDisplay/displayProperties.h"

void displayLightComponent(Editor& editor, Light& dataMember) {
	DisplayProperty<Light::Type>(editor, "Type", dataMember.type);

	ImGui::Separator();

	DisplayProperty<Color>(editor, "Color", dataMember.color);
	ImGui::DragFloat("Intensity", &dataMember.intensity);

	if (dataMember.intensity < 0.f) {
		dataMember.intensity = 0.f;
	}

	switch (dataMember.type) {
	case Light::Type::Directional:
		ImGui::TextWrapped("Directional light has no 'position' and shines all object in a given .. direction. (lol). Direction is derived from object front.");
		break;
	case Light::Type::Spotlight: {
		ImGui::TextWrapped("Spotlight is a combination of directional light and point light, shining a cone subsection in a specific direction.");

		float outerAngle = static_cast<float>(toDegree(dataMember.outerCutOffAngle));
		ImGui::SliderFloat("Outer angle", &outerAngle, 0.f, 90.f);

		float innerAngle = static_cast<float>(toDegree(dataMember.cutOffAngle));
		ImGui::SliderFloat("Inner angle", &innerAngle, 0.f, 90.f);

		if (innerAngle > outerAngle) {
			innerAngle = outerAngle - 0.001f;
			if (innerAngle < 0.f) innerAngle = 0.f;
		}

		dataMember.outerCutOffAngle = toRadian(outerAngle);
		dataMember.cutOffAngle = toRadian(innerAngle);

		DisplayProperty<float>(editor, "Radius", dataMember.radius);

		break;
	}
	case Light::Type::PointLight:
		ImGui::TextWrapped("Point light shines all object in a given radius from a position");
		DisplayProperty<float>(editor, "Radius", dataMember.radius);
		break;
	}
	
	ImGui::SeparatorText("Shadow configuration");
	ImGui::TextWrapped("Shadows are costly! Make sure to limit the number of shadow caster and prefer spotlight over pointlight.");

	DisplayProperty<bool>(editor, "Cast shadow?", dataMember.shadowCaster);

	switch (dataMember.type) {
	case Light::Type::Directional:
		ImGui::TextWrapped("Note: You can only have 1 active directional light shadow caster in a scene.");
		DisplayProperty<float>(editor, "Shadow's near plane", dataMember.shadowNearPlane);
		DisplayProperty<float>(editor, "Shadow's far plane", dataMember.shadowFarPlane);
		break;

	case Light::Type::Spotlight:
		break;

	case Light::Type::PointLight:
		if (dataMember.shadowCaster) ImGui::TextWrapped("[!] Consider using spotlight as shadow caster instead! A point light is basically 6 times of a spotlight.");
		break;
	}
}