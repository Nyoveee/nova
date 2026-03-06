#include <glm/gtc/type_ptr.hpp>

#include "imgui.h"
#include "component.h"

#include "Editor/ComponentInspection/PropertyDisplay/displayProperties.h"

#include "magic_enum.hpp"

#include "ResourceManager/resourceManager.h"
#include "Editor/editor.h"

template<>
inline void DisplayProperty<ParticleEmissionTypeSelection>(Editor& editor, const char*, ParticleEmissionTypeSelection& dataMember) {
	DisplayProperty<ParticleEmissionTypeSelection::EmissionShape>(editor, "Emission Shape", dataMember.emissionShape);

	if (dataMember.emissionShape != ParticleEmissionTypeSelection::EmissionShape::Point) {
		ImGui::BeginChild("", ImVec2(0, 200), ImGuiChildFlags_Border);
		switch (dataMember.emissionShape) {
		case ParticleEmissionTypeSelection::EmissionShape::Cube:
			DisplayProperty<glm::vec3>(editor, "Min", dataMember.cubeEmitter.min);
			DisplayProperty<glm::vec3>(editor, "Max", dataMember.cubeEmitter.max);
			break;
		case ParticleEmissionTypeSelection::EmissionShape::Cone:
			DisplayProperty<float>(editor, "Arc", dataMember.coneEmitter.arc);
			DisplayProperty<float>(editor, "Distance", dataMember.coneEmitter.distance);
			DisplayProperty<float>(editor, "Radius", dataMember.radiusEmitter.radius);
			break;
		case ParticleEmissionTypeSelection::EmissionShape::Sphere:
		case ParticleEmissionTypeSelection::EmissionShape::Edge:
		case ParticleEmissionTypeSelection::EmissionShape::Circle:
		case ParticleEmissionTypeSelection::EmissionShape::Hemisphere:
			DisplayProperty<float>(editor, "Radius", dataMember.radiusEmitter.radius);
			break;
		}
		ImGui::EndChild();
	}
}

//
//template<>
//inline void DisplayProperty<ParticleColorSelection>(Editor& editor, const char*, ParticleColorSelection& dataMember) {
//	ImGui::BeginChild("Particle Color", ImVec2(0, 200), ImGuiChildFlags_Border);
//	DisplayProperty<ColorA>(editor, "Color", dataMember.color);
//	DisplayProperty<glm::vec3>(editor, "Color Offset Min", dataMember.colorOffsetMin);
//	DisplayProperty<glm::vec3>(editor, "Color Offset Max", dataMember.colorOffsetMax);
//	DisplayProperty<float>(editor, "Emissive Multiplier", dataMember.emissiveMultiplier);
//	ImGui::EndChild();
//}
//
//template<>
//inline void DisplayProperty<SizeOverLifetime>(Editor& editor, const char* dataMemberName, SizeOverLifetime& dataMember) {
//	DisplayProperty<bool>(editor, dataMemberName, dataMember.selected);
//
//	if (dataMember.selected) {
//		ImGui::BeginChild("", ImVec2(0, 100), ImGuiChildFlags_Border);
//		DisplayProperty<InterpolationType>(editor, "InterpolationType", dataMember.interpolationType);
//		DisplayProperty<float>(editor, "EndSize", dataMember.endSize);
//		ImGui::EndChild();
//	}
//}
//
//template<>
//inline void DisplayProperty<ColorOverLifetime>(Editor& editor, const char* dataMemberName, ColorOverLifetime& dataMember) {
//	DisplayProperty<bool>(editor, dataMemberName, dataMember.selected);
//
//	if (dataMember.selected) {
//		ImGui::BeginChild("", ImVec2(0, 100), ImGuiChildFlags_Border);
//		DisplayProperty<InterpolationType>(editor, "InterpolationType", dataMember.interpolationType);
//		DisplayProperty<ColorA>(editor, "EndColor", dataMember.endColor);
//		ImGui::EndChild();
//	}
//}
//
//template<>
//inline void DisplayProperty<Trails>(Editor& editor, const char* dataMemberName, Trails& dataMember) {
//	DisplayProperty<bool>(editor, dataMemberName, dataMember.selected);
//	if (dataMember.selected) {
//		ImGui::BeginChild("", ImVec2(0, 300), ImGuiChildFlags_Border);
//		DisplayProperty<TypedResourceID<Texture>>(editor, "Trail Texture", dataMember.trailTexture);
//		DisplayProperty<float>(editor, "Distance Per Emission", dataMember.distancePerEmission);
//		DisplayProperty<float>(editor, "Trail Size", dataMember.trailSize);
//		DisplayProperty<ColorA>(editor, "Trail Color", dataMember.trailColor);
//		DisplayProperty<glm::vec3>(editor, "Color Offset Min", dataMember.trailColorOffsetMin);
//		DisplayProperty<glm::vec3>(editor, "Color Offset Max", dataMember.trailColorOffsetMax);
//		DisplayProperty<float>(editor, "Trail Color Emissive Multiplier", dataMember.trailEmissiveMultiplier);
//		ImGui::EndChild();
//	}
//}

/******************************************************************************
	Particles System
******************************************************************************/
#if 0
struct CubeEmitter {
	glm::vec3 min = { -5.f,-5.f,-5.f };
	glm::vec3 max = { 5.f,5.f,5.f };
};

struct ConeEmitter {
	float arc = 30.f;
	float distance = 5.f;
};

struct RadiusEmitter {
	float radius = 5.f;
};

struct ParticleEmissionTypeSelection {
	enum class EmissionShape {
		Point,
		Sphere,
		Cube,
		Edge,
		Circle,
		Hemisphere,
		Cone
	} emissionShape = EmissionShape::Point;
	RadiusEmitter radiusEmitter;
	CubeEmitter cubeEmitter;
	ConeEmitter coneEmitter;
};

struct ParticleColorSelection {
	ColorA color{ 1.f, 1.f, 1.f,1.f };
	glm::vec3 colorOffsetMin{};
	glm::vec3 colorOffsetMax{};
	float emissiveMultiplier = 1.f;
};

struct SizeOverLifetime {
	bool selected{};
	InterpolationType interpolationType{ InterpolationType::Linear };
	float endSize{};
};

struct ColorOverLifetime {
	bool selected{};
	InterpolationType interpolationType{ InterpolationType::Linear };
	ColorA endColor{};
};

struct Trails {
	bool selected{ false };
	TypedResourceID<Texture> trailTexture;
	float distancePerEmission{ 0.1f };
	float trailSize{ 0.1f };
	ColorA trailColor{ ColorA{1.f,1.f,1.f,1.f} };
	glm::vec3 trailColorOffsetMin{};
	glm::vec3 trailColorOffsetMax{};
	float trailEmissiveMultiplier{ 1.f };
};

struct ParticleEmitter
{
	// Update
	float currentContinuousTime{};
	float currentBurstTime{};
	glm::vec3 prevPosition{};
	bool b_firstPositionUpdate{ true };
	// Categories
	TypedResourceID<Texture> texture{};
	ParticleEmissionTypeSelection particleEmissionTypeSelection{};
	ParticleColorSelection particleColorSelection{};
	SizeOverLifetime sizeOverLifetime{};
	ColorOverLifetime colorOverLifetime{};
	Trails trails{};
	// Core
	bool looping = true;
	bool randomizedDirection = false;
	bool invertMovement = false;
	float startSize = 1.f;
	float minStartSizeOffset = 0.f;
	float maxStartSizeOffset = 0.f;
	float startSpeed = 1;
	glm::vec3 force{};
	// Rotation
	float initialRotation{};
	bool velocityBasedInitialRotation{};
	float minInitialRotationOffset;
	float maxInitialRotationOffset;
	// Angular Velocity
	float initialAngularVelocity{};
	float minAngularVelocityOffset{};
	float maxAngularVelocityOffset{};
	// LifeTime
	float lifeTime = 1;
	float minLifeTimeOffset;
	float maxLifeTimeOffset;

	float particleRate = 100;
	float burstRate = 0;
	int burstAmount = 30;
	// Light
	float lightIntensity{};
	glm::vec3 lightattenuation = glm::vec3{ 1.f, 0.09f, 0.032f };
	float lightRadius{};
};
#endif

void displayParticleEmitterComponent(Editor& editor, ParticleEmitter& emitter) {
	if (ImGui::TreeNode("Basic")) {
		DisplayProperty<TypedResourceID<Texture>>(editor, "Texture", emitter.texture);

		enum SpawnType {
			Time,
			Distance
		} spawnType = emitter.trails.selected ? Distance : Time;

		DisplayProperty<SpawnType>(editor, "Emission Type", spawnType);
		emitter.trails.selected = spawnType == Distance;

		if (emitter.trails.selected) {
			DisplayProperty<float>(editor, "Distance Per Emission", emitter.trails.distancePerEmission);
			DisplayProperty<float>(editor, "-ve distance variance", emitter.trails.minDistanceOffset);
			DisplayProperty<float>(editor, "+ve distance variance", emitter.trails.maxDistanceOffset);
		}
		else {
			DisplayProperty<float>(editor, "Particle Rate", emitter.particleRate);
		}

		DisplayProperty<bool>(editor, "Looping?", emitter.looping);

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Lifetime")) {
		DisplayProperty<float>(editor, "Lifetime", emitter.lifeTime);
		DisplayProperty<float>(editor, "-ve lifetime variance", emitter.minLifeTimeOffset);
		DisplayProperty<float>(editor, "+ve lifetime variance", emitter.maxLifeTimeOffset);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Emission Shape")) {
		DisplayProperty<ParticleEmissionTypeSelection>(editor, "Shape", emitter.particleEmissionTypeSelection);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Burst Emission")) {
		DisplayProperty<float>(editor, "Burst Rate", emitter.burstRate);
		DisplayProperty<int>(editor, "Burst Interval", emitter.burstAmount);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Color Modifier")) {
		Color minOffset = emitter.particleColorSelection.colorOffsetMin;
		Color maxOffset = emitter.particleColorSelection.colorOffsetMax;

		DisplayProperty<ColorA>(editor, "Color", emitter.particleColorSelection.color);
		DisplayProperty<Color>(editor, "-ve color variance", minOffset);
		DisplayProperty<Color>(editor, "+ve color variance", maxOffset);

		DisplayProperty<float>(editor, "Emissive multiplier", emitter.particleColorSelection.emissiveMultiplier);

		emitter.particleColorSelection.colorOffsetMin = minOffset;
		emitter.particleColorSelection.colorOffsetMax = maxOffset;

		DisplayProperty<bool>(editor, "Color over lifetime?", emitter.colorOverLifetime.selected);
		ImGui::BeginDisabled(!emitter.colorOverLifetime.selected);

		DisplayProperty<ColorA>(editor, "End color", emitter.colorOverLifetime.endColor);
		DisplayProperty<InterpolationType>(editor, "Lerp type", emitter.colorOverLifetime.interpolationType);

		ImGui::EndDisabled();

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Light")) {
		DisplayProperty<float>(editor, "Intensity", emitter.lightIntensity);
		DisplayProperty<float>(editor, "Radius", emitter.lightRadius);
		DisplayProperty<glm::vec3>(editor, "Attenutation", emitter.lightattenuation);
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Size")) {
		DisplayProperty<float>(editor, "Initial Size", emitter.startSize);
		DisplayProperty<float>(editor, "-ve size variance", emitter.minStartSizeOffset);
		DisplayProperty<float>(editor, "+ve size variance", emitter.maxStartSizeOffset);

		DisplayProperty<bool>(editor, "Size over lifetime?", emitter.sizeOverLifetime.selected);

		ImGui::BeginDisabled(!emitter.sizeOverLifetime.selected);

		DisplayProperty<float>(editor, "Final size", emitter.sizeOverLifetime.endSize);
		DisplayProperty<InterpolationType>(editor, "Lerp type", emitter.sizeOverLifetime.interpolationType);

		ImGui::EndDisabled();

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Rotation")) {
		DisplayProperty<float>(editor, "Initial rotation", emitter.initialRotation);
		DisplayProperty<float>(editor, "-ve rotation variance", emitter.minStartSizeOffset);
		DisplayProperty<float>(editor, "+ve rotation variance", emitter.maxStartSizeOffset);
		
		DisplayProperty<bool>(editor, "Align rotation to velocity?", emitter.velocityBasedInitialRotation);
		
		DisplayProperty<float>(editor, "Initial angular velocity", emitter.initialAngularVelocity);
		DisplayProperty<float>(editor, "-ve angular rotation variance", emitter.minAngularVelocityOffset);
		DisplayProperty<float>(editor, "+ve angular rotation variance", emitter.maxAngularVelocityOffset);

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Velocity")) {
		DisplayProperty<float>(editor, "Start Speed", emitter.startSpeed);

		DisplayProperty<bool>(editor, "Randomize direction?", emitter.randomizedDirection);
		DisplayProperty<bool>(editor, "Invert direction?", emitter.invertMovement);

		DisplayProperty<glm::vec3>(editor, "Force", emitter.force);
		ImGui::TreePop();
	}

	//	sizeOverLifetime,
	//	colorOverLifetime,
}