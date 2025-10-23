#pragma once

// Instead of following the default constraint templates, we deal with these deals specifically separately.
#if 0
template<>
inline void DisplayProperty<std::vector<ScriptData>>(Editor& editor, const char* dataMemberName, std::vector<ScriptData>& dataMember) {
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
		else if (ImGui::CollapsingHeader(scriptAsset->getClassName().c_str(), &keepScript)) {
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
inline void DisplayProperty<std::unordered_map<std::string, AudioData>>(Editor& editor, const char* dataMemberName, std::unordered_map<std::string, AudioData>& dataMember) {
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
inline void DisplayProperty<ParticleEmissionTypeSelection>(Editor& editor, const char* dataMemberName, ParticleEmissionTypeSelection& dataMember) {
	(void)dataMemberName;
	DisplayProperty<ParticleEmissionTypeSelection::EmissionShape>(propertyReferences, "Emission Shape", dataMember.emissionShape);
	if (dataMember.emissionShape != ParticleEmissionTypeSelection::EmissionShape::Point) {
		ImGui::BeginChild("", ImVec2(0, 200), ImGuiChildFlags_Border);
		switch (dataMember.emissionShape) {
		case ParticleEmissionTypeSelection::EmissionShape::Cube:
			DisplayProperty<glm::vec3>(propertyReferences, "Min", dataMember.cubeEmitter.min);
			DisplayProperty<glm::vec3>(propertyReferences, "Max", dataMember.cubeEmitter.max);
			break;
		case ParticleEmissionTypeSelection::EmissionShape::Cone:
			DisplayProperty<float>(propertyReferences, "Arc", dataMember.coneEmitter.arc);
			DisplayProperty<float>(propertyReferences, "Distance", dataMember.coneEmitter.distance);
			DisplayProperty<float>(propertyReferences, "Radius", dataMember.radiusEmitter.radius);
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

template<>
inline void DisplayProperty<ParticleColorSelection>(Editor& editor, const char* dataMemberName, ParticleColorSelection& dataMember) {
	(void)dataMemberName;
	DisplayProperty<bool>(propertyReferences, "Randomized Color", dataMember.randomizedColor);
	if (!dataMember.randomizedColor) {
		ImGui::BeginChild("", ImVec2(0, 75), ImGuiChildFlags_Border);
		DisplayProperty<ColorA>(propertyReferences, dataMemberName, dataMember.color);
		ImGui::EndChild();
	}
}

template<>
inline void DisplayProperty<SizeOverLifetime>(Editor& editor, const char* dataMemberName, SizeOverLifetime& dataMember) {
	DisplayProperty<bool>(propertyReferences, dataMemberName, dataMember.selected);
	if (dataMember.selected) {
		ImGui::BeginChild("", ImVec2(0, 100), ImGuiChildFlags_Border);
		DisplayProperty<InterpolationType>(propertyReferences, "InterpolationType", dataMember.interpolationType);
		DisplayProperty<float>(propertyReferences, "EndSize", dataMember.endSize);
		ImGui::EndChild();
	}
}

template<>
inline void DisplayProperty<ColorOverLifetime>(Editor& editor, const char* dataMemberName, ColorOverLifetime& dataMember) {
	DisplayProperty<bool>(propertyReferences, dataMemberName, dataMember.selected);
	if (dataMember.selected) {
		ImGui::BeginChild("", ImVec2(0, 100), ImGuiChildFlags_Border);
		DisplayProperty<InterpolationType>(propertyReferences, "InterpolationType", dataMember.interpolationType);
		DisplayProperty<ColorA>(propertyReferences, "EndColor", dataMember.endColor);
		ImGui::EndChild();
	}
}
#endif