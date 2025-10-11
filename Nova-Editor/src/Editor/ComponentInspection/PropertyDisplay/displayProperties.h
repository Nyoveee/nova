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
/***************************************************************************************
	Reference for display properties
****************************************************************************************/
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
/***************************************************************************************
	Property Sub Infos
****************************************************************************************/
void displayMaterialUI(Material& material, ComponentInspector& componentInspector);
void displayScriptFields(ScriptData& scriptData, PropertyReferences& propertyReferences);

/***************************************************************************************
	Base template(Does Nothing)
****************************************************************************************/
template<typename DataMemberType>
inline void DisplayProperty(PropertyReferences& propertyReferences, const char* dataMemberName, auto& dataMember) { (void)propertyReferences, dataMemberName, dataMember; }

/***************************************************************************************
	here we gooooooooooooooooooooooooooooooo! time to list down all the primitives!
****************************************************************************************/
template<IsTypedResourceID DataMemberType>
inline void DisplayProperty(PropertyReferences& propertyReferences, const char* dataMemberName, auto& dataMember) {
	// dataMember is of type TypedAssetID<T>
	using OriginalAssetType = DataMemberType::AssetType;

	propertyReferences.editor.displayAssetDropDownList<OriginalAssetType>(dataMember, dataMemberName, [&](ResourceID resourceId) {
		dataMember = DataMemberType{ resourceId };
	});
}
template<IsEnum DataMemberType>
inline void DisplayProperty(PropertyReferences& propertyReferences, const char* dataMemberName, auto& dataMember) {
	(void)propertyReferences;
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
inline void DisplayProperty<int>(PropertyReferences& propertyReferences, const char* dataMemberName, int& dataMember) {
	(void)propertyReferences;
	ImGui::InputInt(dataMemberName, &dataMember);
}
template<>
inline void DisplayProperty<float>(PropertyReferences& propertyReferences, const char* dataMemberName, float& dataMember) {
	(void)propertyReferences;
	ImGui::InputFloat(dataMemberName, &dataMember);
}
template<>
inline void DisplayProperty<bool>(PropertyReferences& propertyReferences, const char* dataMemberName, bool& dataMember) {
	(void)propertyReferences;
	ImGui::Checkbox(dataMemberName, &dataMember);
}
template<>
inline void DisplayProperty<std::string>(PropertyReferences& propertyReferences, const char* dataMemberName, std::string& dataMember) {
	(void)propertyReferences;
	ImGui::PushTextWrapPos();
	ImGui::Text(dataMemberName);
	ImGui::InputText("##", &dataMember);
	ImGui::PopTextWrapPos();
}
template<>
inline void DisplayProperty<Color>(PropertyReferences& propertyReferences, const char* dataMemberName, Color& dataMember) {
	(void)propertyReferences;
	ImGui::Text(dataMemberName);
	glm::vec3 vec = dataMember;
	ImGui::ColorEdit3("##", glm::value_ptr(vec));
	dataMember = vec;
}
template<>
inline void DisplayProperty<glm::quat>(PropertyReferences& propertyReferences, const char* dataMemberName, glm::quat& dataMember) {
	(void)propertyReferences;
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
inline void DisplayProperty<EulerAngles>(PropertyReferences& propertyReferences, const char* dataMemberName, EulerAngles& dataMember) {
	(void)propertyReferences;
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
inline void DisplayProperty<Radian>(PropertyReferences& propertyReferences, const char* dataMemberName, Radian& dataMember) {
	(void)propertyReferences;
	// convert from radian to degrees for display..
	float angle = static_cast<float>(dataMember);
	angle = toDegree(angle);
	if (ImGui::SliderFloat(dataMemberName, &angle, 0.f, 360.f)) {
		dataMember = toRadian(angle);
	}
}
template<>
inline void DisplayProperty<ResourceID>(PropertyReferences& propertyReferences, const char* dataMemberName, ResourceID& dataMember) {
	ImGui::Text(dataMemberName);

	if (!propertyReferences.resourceManager.doesResourceExist(dataMember)) {
		ImGui::Text("This asset id [%zu] is invalid!", static_cast<std::size_t>(dataMember));
	}
	else {
		ImGui::Text("Asset ID: [%zu]", static_cast<std::size_t>(dataMember));
	}
}
template<>
inline void DisplayProperty<glm::vec3>(PropertyReferences& propertyReferences, const char* dataMemberName, glm::vec3& dataMember) {
	(void)propertyReferences;
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
inline void DisplayProperty<std::unordered_map<MaterialName, Material>>(PropertyReferences& propertyReferences, const char* dataMemberName, std::unordered_map<MaterialName, Material>& dataMember) {
	(void)dataMemberName;
	int i = 0;
	for (auto&& [name, material] : dataMember) {
		ImGui::PushID(i);

		if (ImGui::TreeNode(std::string{ "Material [" + std::to_string(i) + "]: " + name }.c_str())) {
			displayMaterialUI(material, propertyReferences.componentInspector);
			ImGui::TreePop();
		}
		++i;
		ImGui::PopID();
	}
}
template<>
inline void DisplayProperty<std::vector<ScriptData>>(PropertyReferences& propertyReferences, const char* dataMemberName, std::vector<ScriptData>& dataMember) {
	(void)dataMemberName;
	std::vector<ScriptData>& scriptDatas{ dataMember };
	ScriptingAPIManager& scriptingAPIManager{ propertyReferences.scriptingAPIManager };

	if (scriptingAPIManager.isNotCompiled()) {
		if (scriptingAPIManager.hasCompilationFailed()) {
			ImGui::Text("Script compilation has failed!!");
		}
		else {
			ImGui::Text("Changes to scripts have been made, recompiling..");
		}
	}

	ImGui::BeginDisabled(scriptingAPIManager.isNotCompiled() || propertyReferences.engine.isInSimulationMode());

	// Adding Scripts
	propertyReferences.componentInspector.displayAvailableScriptDropDownList(scriptDatas, [&](ResourceID resourceId) {
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

		auto&& [scriptAsset, _] = propertyReferences.resourceManager.getResource<ScriptAsset>(scriptData.scriptId);

		if (!scriptAsset) {
			Logger::warn("Invalid script found, removing it..");
			keepScript = false;
		}
		else if (ImGui::CollapsingHeader(scriptAsset->getClassName().c_str(), &keepScript)){
			displayScriptFields(scriptData, propertyReferences);
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
inline void DisplayProperty<entt::entity>(PropertyReferences& propertyReferences, const char* dataMemberName, entt::entity& dataMember) {
	(void)propertyReferences;
	ImGui::Text("%s: %u", dataMemberName, dataMember);
}
template<>
inline void DisplayProperty<std::unordered_map<std::string, AudioData>>(PropertyReferences& propertyReferences, const char* dataMemberName, std::unordered_map<std::string, AudioData>& dataMember) {
	(void)dataMemberName;
	// Add Audio
	propertyReferences.editor.displayAssetDropDownList<Audio>(std::nullopt, "Add Audio File", [&](ResourceID resourceId)
	{
		// Store full AudioData directly in the component
		auto namePtr = propertyReferences.assetManager.getName(resourceId);

		if (namePtr)
			dataMember.emplace(namePtr->c_str(), AudioData{ resourceId, 1.0f, false });
	});
	// List of Audio Files
	int i{};
	ImGui::BeginChild("", ImVec2(0, 200), ImGuiChildFlags_Border);
	for (auto it = dataMember.begin(); it != dataMember.end();) {
		ImGui::PushID(i++);
		bool keepAudioFile = true;
		auto&& [name, audioData] = *it;
		auto&& [audioAsset, _] = propertyReferences.resourceManager.getResource<Audio>(audioData.AudioId);
		if (!audioAsset) {
			// Invalid ResourceID
			Logger::warn("Invalid Audio ResourceID: {}", static_cast<std::size_t>(audioData.AudioId));
			ImGui::PopID();
			it = dataMember.erase(it);
			continue;
		}
		if (ImGui::CollapsingHeader(name.c_str(), &keepAudioFile)) {
			if (ImGui::Button("Play Audio")) {
				if (propertyReferences.audioSystem.isBGM(audioData.AudioId)) {
					propertyReferences.audioSystem.playBGM(audioData.AudioId, audioData.Volume);
				}
				else {
					propertyReferences.audioSystem.playSFX(audioData.AudioId, 0.f, 0.f, 0.f, audioData.Volume);
				}
			}

			if (ImGui::DragFloat("Adjust Volume", &audioData.Volume, 0.10f, 0.0f, 2.0f, "%.2f")) {
				propertyReferences.audioSystem.AdjustVol(audioData.AudioId, audioData.Volume);
			}

			if (ImGui::Button("Stop Audio")) {
				propertyReferences.audioSystem.StopAudio(audioData.AudioId);
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
inline void DisplayProperty<ParticleEmissionTypeSelection>(PropertyReferences& propertyReferences, const char* dataMemberName, ParticleEmissionTypeSelection& dataMember) {
	(void)dataMemberName;
	DisplayProperty<ParticleEmissionTypeSelection::EmissionShape>(propertyReferences, "Emission Shape", dataMember.emissionShape);
	if (dataMember.emissionShape != ParticleEmissionTypeSelection::EmissionShape::Point) {
		ImGui::BeginChild("", ImVec2(0, 200), ImGuiChildFlags_Border);
		switch (dataMember.emissionShape) {
		case ParticleEmissionTypeSelection::EmissionShape::Cube:
			DisplayProperty<glm::vec3>(propertyReferences, "Min", dataMember.cubeEmitter.min);
			DisplayProperty<glm::vec3>(propertyReferences, "Max", dataMember.cubeEmitter.max);
			break;
		case ParticleEmissionTypeSelection::EmissionShape::Sphere:
		case ParticleEmissionTypeSelection::EmissionShape::Edge:
		case ParticleEmissionTypeSelection::EmissionShape::Circle:
		case ParticleEmissionTypeSelection::EmissionShape::Hemisphere:
			DisplayProperty<float>(propertyReferences, "Radius", dataMember.radiusEmitter.radius);
			break;
		}
		ImGui::EndChild();
	}
	
}