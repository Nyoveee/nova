#pragma once
#include "Editor/editor.h"
#include "Engine/ScriptingAPIManager.h"
#include "Audio/audioSystem.h"
#include "Engine/engine.h"
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include "misc/cpp/imgui_stdlib.h"
#include "reflection.h"
#include "magic_enum.hpp"
#include "type_concepts.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

template <class...>
struct False : std::bool_constant<false> {};

/***************************************************************************************
	Reference for display properties
****************************************************************************************/
#if 0
struct PropertyReferences {
	entt::entity entity;
	ComponentInspector& componentInspector;
	ResourceManager& resourceManager;
	AssetManager& assetManager;
	AudioSystem& audioSystem;
	ScriptingAPIManager& scriptingAPIManager;
	Engine& engine;
	Editor& editor;
	ECS& ecs;
};
#endif

/***************************************************************************************
	Property Sub Infos
****************************************************************************************/
#if 0
void displayMaterialUI(Material& material, ComponentInspector& componentInspector);
#endif
void displayScriptFields(ScriptData& scriptData, Editor& editor);

/***************************************************************************************
	Base template, throws an error.
****************************************************************************************/
template<typename DataMemberType>
inline void DisplayProperty(Editor& editor, const char* dataMemberName, auto& dataMember) {
	static_assert(False<DataMemberType>{}, "Not all types accorded for." __FUNCSIG__);
}

/***************************************************************************************
	Excluded List, See type_concepts.h
****************************************************************************************/
template<NonSerializableTypes DataMemberType>
inline void DisplayProperty(Editor&, const char*, auto&) {};

/***************************************************************************************
	here we gooooooooooooooooooooooooooooooo! time to list down all the primitives!
****************************************************************************************/
template<IsTypedResourceID DataMemberType>
inline void DisplayProperty(Editor& editor, const char* dataMemberName, auto& dataMember) {
	// dataMember is of type TypedAssetID<T>
	using OriginalAssetType = DataMemberType::AssetType;

	editor.displayAssetDropDownList<OriginalAssetType>(dataMember, dataMemberName, [&](ResourceID resourceId) {
		dataMember = DataMemberType{ resourceId };
	});
}

template<IsEnum DataMemberType>
inline void DisplayProperty(Editor&, const char* dataMemberName, auto& dataMember) {
	// it's an enum. let's display a dropdown box for this enum.
	// how? using enum reflection provided by "magic_enum.hpp" :D
	// get the list of all possible enum values
	constexpr auto listOfEnumValues = magic_enum::enum_entries<DataMemberType>();

	if (ImGui::BeginCombo(dataMemberName, std::string{ magic_enum::enum_name<DataMemberType>(dataMember) }.c_str())) {
		for (auto&& [enumValue, enumInString] : listOfEnumValues) {
			if (ImGui::Selectable(std::string{ enumInString }.c_str(), enumValue == dataMember)) {
				dataMember = enumValue;
			}
		}
		ImGui::EndCombo();
	}
}
template<>
inline void DisplayProperty<int>(Editor&, const char* dataMemberName, int& dataMember) {
	ImGui::InputInt(dataMemberName, &dataMember);
}

template<>
inline void DisplayProperty<unsigned int>(Editor&, const char* dataMemberName, unsigned int& dataMember) {
	ImGui::InputScalar(dataMemberName, ImGuiDataType_U32, &dataMember);
}

template<>
inline void DisplayProperty<double>(Editor&, const char* dataMemberName, double& dataMember) {
	ImGui::InputDouble(dataMemberName, &dataMember);
}

template<>
inline void DisplayProperty<float>(Editor&, const char* dataMemberName, float& dataMember) {
	ImGui::InputFloat(dataMemberName, &dataMember);
}

template<>
inline void DisplayProperty<bool>(Editor&, const char* dataMemberName, bool& dataMember) {
	ImGui::Checkbox(dataMemberName, &dataMember);
}

template<>
inline void DisplayProperty<std::string>(Editor&, const char* dataMemberName, std::string& dataMember) {
	ImGui::PushTextWrapPos();
	ImGui::Text(dataMemberName);
	ImGui::InputText("##", &dataMember);
	ImGui::PopTextWrapPos();
}

template<>
inline void DisplayProperty<Color>(Editor&, const char* dataMemberName, Color& dataMember) {
	ImGui::Text(dataMemberName);
	glm::vec3 vec = dataMember;
	ImGui::ColorEdit3("##", glm::value_ptr(vec));
	dataMember = vec;
}

template<>
inline void DisplayProperty<ColorA>(Editor&, const char* dataMemberName, ColorA& dataMember) {
	ImGui::Text(dataMemberName);
	glm::vec4 vec = dataMember;
	ImGui::ColorEdit4("##", glm::value_ptr(vec));
	dataMember = vec;
}

template<>
inline void DisplayProperty<glm::quat>(Editor&, const char* dataMemberName, glm::quat& dataMember) {
	if (ImGui::BeginTable("MyTable", 5, ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_NoPadInnerX)) {
		ImGui::TableSetupColumn("Fixed Column", ImGuiTableColumnFlags_WidthFixed, 100.0f);
		ImGui::TableSetupColumn("Stretch Column", ImGuiTableColumnFlags_WidthStretch);

		ImGui::TableNextRow();

		ImGui::TableNextColumn();
		ImGui::Text((std::string{ dataMemberName } + "\n(quarternion)").c_str());

		ImGui::BeginDisabled();
		ImGui::TableNextColumn();
		ImGui::SliderFloat("a", &dataMember.w, -1, 1);

		ImGui::TableNextColumn();
		ImGui::SliderFloat("bi", &dataMember.x, -1, 1);

		ImGui::TableNextColumn();
		ImGui::SliderFloat("cj", &dataMember.y, -1, 1);

		ImGui::TableNextColumn();
		ImGui::SliderFloat("dk", &dataMember.z, -1, 1);
		ImGui::EndDisabled();

		ImGui::EndTable();
	}

	if (ImGui::Button(ICON_FA_RECYCLE " Reset")) {
		dataMember = {};
	}
}
template<>
inline void DisplayProperty<EulerAngles>(Editor&, const char* dataMemberName, EulerAngles& dataMember) {
	// convert from radian to degrees for display..
	glm::vec3 eulerAngles = static_cast<glm::vec3>(dataMember);
	eulerAngles.x = toDegree(eulerAngles.x);
	eulerAngles.y = toDegree(eulerAngles.y);
	eulerAngles.z = toDegree(eulerAngles.z);

	if (ImGui::BeginTable("MyTable", 4, ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_NoPadInnerX)) {
		ImGui::TableSetupColumn("Fixed Column", ImGuiTableColumnFlags_WidthFixed, 100.0f);
		ImGui::TableSetupColumn("Stretch Column", ImGuiTableColumnFlags_WidthStretch);

		ImGui::TableNextRow();

		ImGui::TableNextColumn();
		ImGui::Text((std::string{ dataMemberName } + "\n(degrees)").c_str());

		ImGui::TableNextColumn();
		ImGui::InputFloat("pitch", &eulerAngles.x);

		ImGui::TableNextColumn();
		ImGui::InputFloat("yaw", &eulerAngles.y);

		ImGui::TableNextColumn();
		ImGui::InputFloat("roll", &eulerAngles.z);

		ImGui::EndTable();
	}

	// Clamp result..
	eulerAngles.x = std::clamp(eulerAngles.x, -180.f, 180.f);
	eulerAngles.y = std::clamp(eulerAngles.y, -90.f, 90.f);
	eulerAngles.z = std::clamp(eulerAngles.z, -180.f, 180.f);
	dataMember = EulerAngles{ {toRadian(eulerAngles.x), toRadian(eulerAngles.y), toRadian(eulerAngles.z)} };
}
template<>
inline void DisplayProperty<Radian>(Editor&, const char* dataMemberName, Radian& dataMember) {
	// convert from radian to degrees for display..
	float angle = static_cast<float>(dataMember);
	angle = toDegree(angle);
	if (ImGui::SliderFloat(dataMemberName, &angle, 0.f, 360.f)) {
		dataMember = toRadian(angle);
	}
}
template<>
inline void DisplayProperty<ResourceID>(Editor& editor, const char* dataMemberName, ResourceID& dataMember) {
	ImGui::Text(dataMemberName);

	if (!editor.resourceManager.doesResourceExist(dataMember)) {
		ImGui::Text("This asset id [%zu] is invalid!", static_cast<std::size_t>(dataMember));
	}
	else {
		ImGui::Text("Asset ID: [%zu]", static_cast<std::size_t>(dataMember));
	}
}

template<>
inline void DisplayProperty<glm::vec4>(Editor&, const char* dataMemberName, glm::vec4& dataMember) {
	if (ImGui::BeginTable("MyTable", 5, ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_NoPadInnerX)) {
		ImGui::TableSetupColumn("Fixed Column", ImGuiTableColumnFlags_WidthFixed, 100.0f);
		ImGui::TableSetupColumn("Stretch Column", ImGuiTableColumnFlags_WidthStretch);

		ImGui::TableNextRow();

		ImGui::TableNextColumn();
		ImGui::Text(dataMemberName);

		ImGui::BeginDisabled();
		ImGui::TableNextColumn();
		ImGui::InputFloat("x", &dataMember.x);

		ImGui::TableNextColumn();
		ImGui::InputFloat("y", &dataMember.y);

		ImGui::TableNextColumn();
		ImGui::InputFloat("z", &dataMember.z);

		ImGui::TableNextColumn();
		ImGui::InputFloat("w", &dataMember.w);
		ImGui::EndDisabled();

		ImGui::EndTable();
	}
}

template<>
inline void DisplayProperty<glm::vec3>(Editor&, const char* dataMemberName, glm::vec3& dataMember) {
	if (ImGui::BeginTable("MyTable", 4, ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_NoPadInnerX)) {
		ImGui::TableSetupColumn("Fixed Column", ImGuiTableColumnFlags_WidthFixed, 100.0f);
		ImGui::TableSetupColumn("Stretch Column", ImGuiTableColumnFlags_WidthStretch);

		ImGui::TableNextRow();

		ImGui::TableNextColumn();
		ImGui::AlignTextToFramePadding();
		ImGui::Text(dataMemberName);

		ImGui::TableNextColumn();
		ImGui::InputFloat("x", &dataMember.x);

		ImGui::TableNextColumn();
		ImGui::InputFloat("y", &dataMember.y);

		ImGui::TableNextColumn();
		ImGui::InputFloat("z", &dataMember.z);

		ImGui::EndTable();
	}
}

template<>
inline void DisplayProperty<glm::vec2>(Editor&, const char* dataMemberName, glm::vec2& dataMember) {
	if (ImGui::BeginTable("MyTable", 3, ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_NoPadInnerX)) {
		ImGui::TableSetupColumn("Fixed Column", ImGuiTableColumnFlags_WidthFixed, 100.0f);
		ImGui::TableSetupColumn("Stretch Column", ImGuiTableColumnFlags_WidthStretch);

		ImGui::TableNextRow();

		ImGui::TableNextColumn();
		ImGui::AlignTextToFramePadding();
		ImGui::Text(dataMemberName);

		ImGui::TableNextColumn();
		ImGui::InputFloat("x", &dataMember.x);

		ImGui::TableNextColumn();
		ImGui::InputFloat("y", &dataMember.y);

		ImGui::EndTable();
	}
}

template<>
inline void DisplayProperty<entt::entity>(Editor&, const char* dataMemberName, entt::entity& dataMember) {
	ImGui::Text("%s: %u", dataMemberName, dataMember);
}

/***************************************************************************************
	Special handling for unique data members.
****************************************************************************************/
template<>
inline void DisplayProperty<std::vector<ScriptData>>(Editor& editor, const char*, std::vector<ScriptData>& dataMember) {
	std::vector<ScriptData>& scriptDatas{ dataMember };
	ScriptingAPIManager& scriptingAPIManager{ editor.engine.scriptingAPIManager };

	if (scriptingAPIManager.isNotCompiled()) {
		if (scriptingAPIManager.hasCompilationFailed()) {
			ImGui::Text("Script compilation has failed!!");
		}
		else {
			ImGui::Text("Changes to scripts have been made, recompiling..");
		}
	}

	ImGui::BeginDisabled(scriptingAPIManager.isNotCompiled() || editor.engine.isInSimulationMode());

	// Adding Scripts
	editor.componentInspector.displayAvailableScriptDropDownList(scriptDatas, [&](ResourceID resourceId) {
		ScriptData scriptData{ resourceId };
		scriptData.fields = scriptingAPIManager.getScriptFieldDatas(scriptData.scriptId);
		scriptDatas.push_back(scriptData);
	});

	int i{};
	// Removal of Scripts
	ImGui::BeginChild("", ImVec2(0, 400), ImGuiChildFlags_Border);
	std::vector<ScriptData>::iterator it = std::remove_if(std::begin(scriptDatas), std::end(scriptDatas), [&](ScriptData& scriptData) {
		ImGui::PushID(i++);
		bool keepScript = true;

		auto&& [scriptAsset, _] = editor.resourceManager.getResource<ScriptAsset>(scriptData.scriptId);

		if (!scriptAsset) {
			Logger::warn("Invalid script found, removing it..");
			keepScript = false;
		}
		else if (ImGui::CollapsingHeader(scriptAsset->getClassName().c_str(), &keepScript)){
			displayScriptFields(scriptData, editor);
		}
		ImGui::PopID();
		return !keepScript;
	});

	ImGui::EndChild();
	if (it != std::end(scriptDatas)) {
		scriptDatas.erase(it);
	}
	ImGui::EndDisabled();
}

template<>
inline void DisplayProperty<std::unordered_map<std::string, AudioData>>(Editor& editor, const char*, std::unordered_map<std::string, AudioData>& dataMember) {
	// Add Audio
	editor.displayAssetDropDownList<Audio>(std::nullopt, "Add Audio File", [&](ResourceID resourceId)
	{
		// Store full AudioData directly in the component
		auto namePtr = editor.assetManager.getName(resourceId);

		if (namePtr)
			dataMember.emplace(namePtr->c_str(), AudioData{ resourceId, 1.0f, false });
	});

	AudioSystem& audioSystem = editor.engine.audioSystem;

	// List of Audio Files
	int i{};

	ImGui::BeginChild("", ImVec2(0, 200), ImGuiChildFlags_Border);
	for (auto it = dataMember.begin(); it != dataMember.end();) {
		ImGui::PushID(i++);
		bool keepAudioFile = true;
		auto&& [name, audioData] = *it;
		auto&& [audioAsset, _] = editor.resourceManager.getResource<Audio>(audioData.AudioId);
		if (!audioAsset) {
			// Invalid ResourceID
			Logger::warn("Invalid Audio ResourceID: {}", static_cast<std::size_t>(audioData.AudioId));
			ImGui::PopID();
			it = dataMember.erase(it);
			continue;
		}

		if (ImGui::CollapsingHeader(name.c_str(), &keepAudioFile)) {
			if (ImGui::Button("Play Audio")) {
				if (audioSystem.isBGM(audioData.AudioId)) {
					audioSystem.playBGM(audioData.AudioId, audioData.Volume);
				}
				else {
					audioSystem.playSFX(audioData.AudioId, 0.f, 0.f, 0.f, audioData.Volume);
				}
			}

			if (ImGui::DragFloat("Adjust Volume", &audioData.Volume, 0.10f, 0.0f, 2.0f, "%.2f")) {
				audioSystem.AdjustVol(audioData.AudioId, audioData.Volume);
			}

			if (ImGui::Button("Stop Audio")) {
				audioSystem.StopAudio(audioData.AudioId);
			}
		}

		ImGui::PopID();

		if (!keepAudioFile) {
			it = dataMember.erase(it);
		}
		else {
			++it;
		}
	}
	ImGui::EndChild();
}
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

template<>
inline void DisplayProperty<ParticleColorSelection>(Editor& editor, const char* dataMemberName, ParticleColorSelection& dataMember) {
	DisplayProperty<bool>(editor, "Randomized Color", dataMember.randomizedColor);

	if (!dataMember.randomizedColor) {
		ImGui::BeginChild("", ImVec2(0, 75), ImGuiChildFlags_Border);
		DisplayProperty<ColorA>(editor, dataMemberName, dataMember.color);
		ImGui::EndChild();
	}
}

template<>
inline void DisplayProperty<SizeOverLifetime>(Editor& editor, const char* dataMemberName, SizeOverLifetime& dataMember) {
	DisplayProperty<bool>(editor, dataMemberName, dataMember.selected);

	if (dataMember.selected) {
		ImGui::BeginChild("", ImVec2(0, 100), ImGuiChildFlags_Border);
		DisplayProperty<InterpolationType>(editor, "InterpolationType", dataMember.interpolationType);
		DisplayProperty<float>(editor, "EndSize", dataMember.endSize);
		ImGui::EndChild();
	}
}

template<>
inline void DisplayProperty<ColorOverLifetime>(Editor& editor, const char* dataMemberName, ColorOverLifetime& dataMember) {
	DisplayProperty<bool>(editor, dataMemberName, dataMember.selected);

	if (dataMember.selected) {
		ImGui::BeginChild("", ImVec2(0, 100), ImGuiChildFlags_Border);
		DisplayProperty<InterpolationType>(editor, "InterpolationType", dataMember.interpolationType);
		DisplayProperty<ColorA>(editor, "EndColor", dataMember.endColor);
		ImGui::EndChild();
	}
}