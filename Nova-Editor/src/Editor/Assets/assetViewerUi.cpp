#include <iterator> 
#include <regex>

#include "assetViewerUi.h"

#include "imgui.h"
#include "IconsFontAwesome6.h"

#include "misc/cpp/imgui_stdlib.h"

#include "magic_enum.hpp"

#include "customShader.h"
#include "Material.h"

#include "Editor/editor.h"

#include "Serialisation/serialisation.h"
#include "Editor/ComponentInspection/displayComponent.h"
#include "Editor/ComponentInspection/PropertyDisplay/displayProperties.h"

#include "systemResource.h"


AssetViewerUI::AssetViewerUI(Editor& editor, AssetManager& assetManager, ResourceManager& resourceManager) :
	editor							{ editor },
	assetManager					{ assetManager },
	resourceManager					{ resourceManager },
	selectedResourceId				{ INVALID_RESOURCE_ID },
	toSerialiseSelectedDescriptor	{ false }
{}

void AssetViewerUI::update() {
#if 1
	ImGui::Begin(ICON_FA_AUDIO_DESCRIPTION " Asset Viewer");

	if (selectedResourceId == INVALID_RESOURCE_ID) {
		ImGui::Text("No resource selected.");
		ImGui::End();
		return;
	}

	auto descriptorPtr = assetManager.getDescriptor(selectedResourceId);

	if (!descriptorPtr) {
		ImGui::Text("Invalid resource id selected.");
		ImGui::End();
		return;
	}

	toSerialiseSelectedDescriptor = false;

	// Display common shared asset info across all assets..
	ImGui::Text("Resource ID: %zu", static_cast<std::size_t>(descriptorPtr->id));

	// ===== Display asset name ======
	ImGui::InputText("Name: ", &selectedResourceName);

	if (ImGui::IsItemDeactivatedAfterEdit()) {
		// empty name is not a valid name.
		if (selectedResourceName == "") {
			selectedResourceName = descriptorPtr->name;
		}
		else {
			toSerialiseSelectedDescriptor = true;
			descriptorPtr->name = selectedResourceName;
		}
	}

	ImGui::Text("Filepath: %s", selectedResourceStemCopy.c_str());

	ImGui::InputText(selectedResourceExtension.empty() ? "##" : selectedResourceExtension.c_str(), &selectedResourceStemCopy);

	if (ImGui::IsItemDeactivatedAfterEdit() && std::filesystem::path{ descriptorPtr->filepath }.stem() != selectedResourceStemCopy) {
		if (!assetManager.renameFile(selectedResourceId, selectedResourceStemCopy)) {
			// reset rename attempt.
			selectedResourceStemCopy = std::filesystem::path{ descriptorPtr->filepath }.stem().string();
		}
		else {
			toSerialiseSelectedDescriptor = true;
		}
	}

	// If there is a name or filepath change, serialise the descriptor..
	if (toSerialiseSelectedDescriptor) {
		if (resourceManager.isResource<ScriptAsset>(selectedResourceId)) {
			updateScriptFilePath(descriptorPtr->filepath, selectedResourceId);
		}

		assetManager.serialiseDescriptor(selectedResourceId);
	}

	auto displayResourceUIFunctor = [&]<ValidResource ...T>(ResourceID id) {
		([&] {
			if (resourceManager.isResource<T>(id)) {
				displayAssetUI<T>(*descriptorPtr);
			}
		}(), ...);
	};

	displayResourceUIFunctor.template operator()<ALL_RESOURCES>(selectedResourceId);
	ImGui::End();
#endif
}

void AssetViewerUI::updateScriptFilePath(AssetFilePath const& filepath, [[maybe_unused]] ResourceID id) {
	std::ifstream inputScriptFile{ filepath };

	std::string newName = std::filesystem::path{ filepath }.stem().string();

	if (!inputScriptFile) {
		Logger::error("Fail to read script file.");
		return;
	}

	// we read the whole file's content into a variable
	std::string scriptContents{ std::istreambuf_iterator<char>(inputScriptFile), std::istreambuf_iterator<char>() };

	// i love regex. ask me if u need regex explaination.
	std::regex classRegex(R"(class [-\w\s]+\s?:?([\w\s]+)\{)");
	scriptContents = std::regex_replace(scriptContents, classRegex, "class " + newName + " :$1{");
	
	std::ofstream outputScriptFile{ filepath };

	if (!outputScriptFile) {
		Logger::error("Fail to open script file for writing.");
		return;
	}

	outputScriptFile << scriptContents;

	Logger::info("Successfully updated script name.");
}

void AssetViewerUI::selectNewResourceId(ResourceID id) {
	selectedResourceId = id;

	auto descriptorPtr = assetManager.getDescriptor(selectedResourceId);

	if (!descriptorPtr) {
		return;
	}

	selectedResourceName = descriptorPtr->name;
	selectedResourceStemCopy = std::filesystem::path{ descriptorPtr->filepath }.stem().string();
	selectedResourceExtension = std::filesystem::path{ descriptorPtr->filepath }.extension().string();

	// save a copy of the current font size..
	if (resourceManager.isResource<Font>(id)) {
		AssetInfo<Font>* fontDescriptorPtr = static_cast<AssetInfo<Font>*>(descriptorPtr);
		copyOfSelectedFontSize = fontDescriptorPtr->fontSize;
	}
	else if (resourceManager.isResource<Model>(id)) {
		AssetInfo<Model>* modelDescriptorPtr = static_cast<AssetInfo<Model>*>(descriptorPtr);
		copyOfScale = modelDescriptorPtr->scale;
	}
	else if (resourceManager.isResource<Prefab>(id)) {
		auto&& prefabMap = editor.engine.prefabManager.getPrefabMap();
		auto iterator = prefabMap.find(selectedResourceId);

		if (iterator == prefabMap.end()) {
			rootPrefabEntity = editor.engine.prefabManager.loadPrefab(selectedResourceId);
		}
		else {
			rootPrefabEntity = iterator->second;
		}

		selectedPrefabEntity = rootPrefabEntity;
	}
}


void AssetViewerUI::displayMaterialInfo([[maybe_unused]] AssetInfo<Material>& descriptor)
{
	if (!ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen)) {
		return;
	}

	auto&& [material, loadStatus] = resourceManager.getResource<Material>(selectedResourceId);

	if (!material) {
		ImGui::Text("Invalid material.");
		return;
	}

	// Chosen Shader
	TypedResourceID<CustomShader> const& shaderId = material->materialData.selectedShader;
	auto&& [customShader, __] = resourceManager.getResource<CustomShader>(shaderId);

	if (!customShader) {
		ImGui::Text("No shader selected.");
	}

	auto updateMaterialProperty = [&](std::unordered_map<std::string, std::string> const& shaderUniforms) {
		material->materialData.overridenUniforms.clear();

		for (auto&& [identifer, type] : shaderUniforms) {
			// thank you zhi wei for writing all of the if cases. :)
			if (type == "bool")
				material->materialData.overridenUniforms.insert({ identifer, { type, bool{} } });
			if (type == "int")
				material->materialData.overridenUniforms.insert({ identifer, { type, int{} } });
			if (type == "uint")
				material->materialData.overridenUniforms.insert({ identifer, { type, unsigned{} } });
			if (type == "float")
				material->materialData.overridenUniforms.insert({ identifer, { type, float{} } });
			if (type == "vec2")
				material->materialData.overridenUniforms.insert({ identifer, { type, glm::vec2{} } });
			if (type == "vec3")
				material->materialData.overridenUniforms.insert({ identifer, { type, glm::vec3{} } });
			if (type == "vec4")
				material->materialData.overridenUniforms.insert({ identifer, { type, glm::vec4{} } });
			if (type == "mat3")
				material->materialData.overridenUniforms.insert({ identifer, { type, glm::mat3{} } });
			if (type == "mat4")
				material->materialData.overridenUniforms.insert({ identifer, { type, glm::mat4{} } });
			if (type == "sampler2D")
				material->materialData.overridenUniforms.insert({ identifer, { type, TypedResourceID<Texture>{ NONE_TEXTURE_ID } } });

			// custom glsl types..
			if (type == "Color")
				material->materialData.overridenUniforms.insert({ identifer, { type, Color{ 1.f, 1.f, 1.f } } });
			if (type == "ColorA")
				material->materialData.overridenUniforms.insert({ identifer, { type, ColorA{ 1.f, 1.f, 1.f, 1.f } } });
			if (type == "NormalizedFloat")
				material->materialData.overridenUniforms.insert({ identifer, { type, NormalizedFloat{} } });
		}
	};

	editor.displayAssetDropDownList<CustomShader>(shaderId, "Shader", [&](ResourceID id) {
		material->materialData.selectedShader = TypedResourceID<CustomShader>{ id };

		// Since we change shader, we need to reupdate the overridenUniforms..
		auto&& [customShader, __] = resourceManager.getResource<CustomShader>(id);

		if (!customShader) {
			assert(false && "The display asset drop down list should never provide an invalid resource id.");
			Logger::error("Selected an invalid shader!");
			return;
		}

		updateMaterialProperty(customShader->customShaderData.uniforms);
	});
	
	if (!customShader) {
		return;
	}

	if (!customShader->getShader() || !customShader->getShader().value().hasCompiled()) {
		ImGui::Text("Invalid shader. Underlying shader failed to compile.");

		if (!customShader->getShader()) {
			ImGui::Text("Syntax error. Custom parser failed to parse.");
		}
		else {
			auto&& shader = customShader->getShader().value();
			ImGui::TextWrapped("%s", shader.getErrorMessage().c_str());
		}

		return;
	}

	if (ImGui::Button("Update Material Properties From Shader")) {
		updateMaterialProperty(customShader->customShaderData.uniforms);
	}
	
	int imguiCounter = 0;

	// Display the current material values..
	for (auto&& [name, uniformData] : material->materialData.overridenUniforms) {
		auto&& [type, uniformValue] = uniformData;

		ImGui::PushID(imguiCounter++);

		std::visit([&](auto&& value) {
			using Type = std::decay_t<decltype(value)>;

			if constexpr (std::same_as<Type, glm::mat3> || std::same_as<Type, glm::mat4>) {
				assert(type == "mat3" || type == "mat4");
			}
			else {
				DisplayProperty<Type>(editor, name.c_str(), value);
			}
		}, uniformValue);

		ImGui::PopID();
	}
}

void AssetViewerUI::displayShaderInfo(AssetInfo<CustomShader>& descriptor) {
	auto&& [shader, loadStatus] = resourceManager.getResource<CustomShader>(selectedResourceId);

	if (!shader) {
		ImGui::Text("Invalid shader.");
		return;
	}
	
	auto const& openglShaderOpt = shader->getShader();

	constexpr auto listOfEnumValues = magic_enum::enum_entries<Pipeline>();

	if (!ImGui::CollapsingHeader("Shader", ImGuiTreeNodeFlags_DefaultOpen)) {
		return;
	}

	if (ImGui::BeginCombo("Pipeline", std::string{ magic_enum::enum_name(descriptor.pipeline) }.c_str())) {
		for (auto&& [enumValue, enumInString] : listOfEnumValues) {
			if (ImGui::Selectable(std::string{ enumInString }.c_str(), enumValue == descriptor.pipeline)) {
				descriptor.pipeline = enumValue;
				shader->customShaderData.pipeline = enumValue;
				
				// serialise immediately..
				assetManager.serializeDescriptor<CustomShader>(selectedResourceId);

				// recompile..
				shader->compile();
			}
		}

		ImGui::EndCombo();
	}

	if (!openglShaderOpt) {
		ImGui::Text("Custom shader failed to parsed. Please check for syntax error.");
	}
	else {
		auto const& openglShader = openglShaderOpt.value();

		if (!openglShader.hasCompiled()) {
			ImGui::BeginChild("Error window", ImVec2{0.f, 0.f}, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY);
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
			ImGui::TextWrapped("%s", openglShader.getErrorMessage().c_str());
			ImGui::PopStyleColor();
			ImGui::EndChild();
		}

		if (ImGui::TreeNode("Vertex Shader")) {
			ImGui::BeginChild("Vertex Shader", ImVec2{}, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY);
			ImGui::Text("%s", openglShader.getVertexShader().c_str());
			ImGui::EndChild();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Fragment Shader")) {
			ImGui::BeginChild("Fragment Shader", ImVec2{}, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY);
			ImGui::Text("%s", openglShader.getFragmentShader().c_str());
			ImGui::EndChild();

			ImGui::TreePop();
		}
	}
}

void AssetViewerUI::displayTextureInfo(AssetInfo<Texture>& textureInfo) {
	ImGui::SeparatorText("Texture");

	constexpr auto listOfEnumValues = magic_enum::enum_entries<AssetInfo<Texture>::Compression>();

	bool toRecompile = false;

	editor.displayEnumDropDownList<AssetInfo<Texture>::TextureType>(textureInfo.type, "Texture Type", [&](auto textureType) {
		if (textureType != textureInfo.type) {
			textureInfo.type = textureType;
			toRecompile = true;

			switch (textureInfo.type)
			{
				using enum AssetInfo<Texture>::TextureType;
				using enum AssetInfo<Texture>::Compression;
			case sRGB:
				textureInfo.compression = BC1_SRGB;
				break;
			case sRGBA:
				textureInfo.compression = BC3_SRGB;
				break;
			case Linear:
				textureInfo.compression = BC1_Linear;
				break;
			case NormalMap:
				textureInfo.compression = BC5;
				break;
			case Custom:
				toRecompile = false;
				break;
			}
		}
	});

	if (textureInfo.type != AssetInfo<Texture>::TextureType::Custom) {
		ImGui::BeginDisabled();
	}

	editor.displayEnumDropDownList<AssetInfo<Texture>::Compression>(textureInfo.compression, "Compression", [&](auto enumValue) {
		if (enumValue != textureInfo.compression) {
			textureInfo.compression = enumValue;
			toRecompile = true;
		}
	});

	if (textureInfo.type != AssetInfo<Texture>::TextureType::Custom) {
		ImGui::EndDisabled();
	}

	if (toRecompile) {
		recompileResourceWithUpdatedDescriptor<Texture>(textureInfo);
	}
}

void AssetViewerUI::displayModelInfo([[maybe_unused]] AssetInfo<Model>& descriptor) {
	auto&& [model, loadStatus] = resourceManager.getResource<Model>(selectedResourceId);

	if (!model) {
		switch (loadStatus)
		{
		case ResourceManager::QueryResult::Invalid:
			ImGui::Text("Resource ID is invalid.");
			return;
		case ResourceManager::QueryResult::WrongType:
			ImGui::Text("This should never happened. Resource ID is not a model?");
			assert(false && "Resource ID is not a model.");
			return;
		case ResourceManager::QueryResult::Loading:
			ImGui::Text("Loading..");
			return;
		case ResourceManager::QueryResult::LoadingFailed:
			ImGui::Text("Loading of model failed.");
			return;
		default:
			assert(false);
			return;
		}
	}

	if (ImGui::CollapsingHeader("Model", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Text("Max dimension: %.2f", model->maxDimension);

		ImGui::InputFloat("Scale", &copyOfScale);

		if (ImGui::IsItemDeactivatedAfterEdit()) {
			if (copyOfScale <= 0.f) {
				// invalid scale..
				copyOfScale = descriptor.scale;
			}
			else {
				// we recompile..
				descriptor.scale = copyOfScale;
				recompileResourceWithUpdatedDescriptor<Model>(descriptor);
				return;
			}
		}

		ImGui::SeparatorText(std::string{ "Materials: " + std::to_string(model->materialNames.size()) }.c_str());

		for (unsigned int i = 0; i < model->materialNames.size(); ++i) {
			std::string const& materialName = model->materialNames[i];
			ImGui::Text("[%u] %s", i, materialName.c_str());
		}

		ImGui::SeparatorText(std::string{ "Sub meshes: " + std::to_string(model->meshes.size()) }.c_str());

		int counter = 0;

		for (auto& mesh : model->meshes) {
			ImGui::PushID(counter);

			ImGui::Text(mesh.name.c_str());
			ImGui::BeginChild("##", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Borders);

			ImGui::Text("Vertices: %zu", mesh.positions.size());
			ImGui::Text("Indices: %zu", mesh.indices.size());

			if (mesh.materialIndex >= model->materialNames.size()) {
				ImGui::Text("Invalid material!");
			}
			else {
				ImGui::Text("Material name: %s", model->materialNames[mesh.materialIndex].c_str());
			}

			if (ImGui::TreeNode("[Debug] Display bone vertex weight.")) {
				if (!mesh.vertexWeights.size()) {
					ImGui::Text("Boneless mesh.");
				}
				else {
					for (std::size_t vertexIndex = 0; vertexIndex < mesh.vertexWeights.size(); ++vertexIndex) {
						auto const& vertexWeights = mesh.vertexWeights[vertexIndex];
						
						ImGui::Text("Vertex %zu: ", vertexIndex);

						for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
							if (vertexWeights.boneIndices[i] == NO_BONE_INDEX) {
								continue;
							}

							ImGui::SameLine();
							ImGui::Text("bone: %hu, weight: %f", vertexWeights.boneIndices[i], vertexWeights.weights[i]);
						}
					}
				}
				ImGui::TreePop();
			}

			ImGui::EndChild();
			ImGui::PopID();
			++counter;
		}


		if (!model->skeleton) {
			ImGui::Text("This mesh has no skeleton.");
			return;
		}

		Skeleton const& skeleton = model->skeleton.value();
		
		ImGui::SeparatorText(std::string{ "Bones: " + std::to_string(skeleton.bones.size()) }.c_str());
		displayBoneHierarchy(skeleton.rootBone, skeleton);

		ImGui::SeparatorText(std::string{ "[Debug] Nodes: " + std::to_string(skeleton.nodes.size()) }.c_str());
		displayNodeHierarchy(skeleton.rootNode, skeleton);
	}

	displayAnimationInfo(descriptor);
}

void AssetViewerUI::displayAnimationInfo([[maybe_unused]] AssetInfo<Model>& descriptor) {
	auto&& [animationResource, loadStatus] = resourceManager.getResource<Model>(selectedResourceId);

	if (!animationResource) {
		switch (loadStatus)
		{
		case ResourceManager::QueryResult::Invalid:
			ImGui::Text("Resource ID is invalid.");
			return;
		case ResourceManager::QueryResult::WrongType:
			ImGui::Text("This should never happened. Resource ID is not a animation?");
			assert(false && "Resource ID is not a animation.");
			return;
		case ResourceManager::QueryResult::Loading:
			ImGui::Text("Loading..");
			return;
		case ResourceManager::QueryResult::LoadingFailed:
			ImGui::Text("Loading of animation failed.");
			return;
		default:
			assert(false);
			return;
		}
	}

	if (ImGui::CollapsingHeader("Animation", ImGuiTreeNodeFlags_DefaultOpen)) {
		for (auto&& animation : animationResource->animations) {
			ImGui::SeparatorText(animation.name.c_str());
			ImGui::Text("Duration (in seconds): %.2f", animation.durationInSeconds);
			ImGui::Text("Duration (in ticks): %.2f", animation.durationInTicks);
			ImGui::Text("Ticks Per Second: %.2f", animation.ticksPerSecond);

			for (auto&& [channelName, channel] : animation.animationChannels) {
				float highestPositionKey = channel.positions.size() ? channel.positions[channel.positions.size() - 1].key : 0.f;
				float highestRotationKey = channel.rotations.size() ? channel.rotations[channel.rotations.size() - 1].key : 0.f;
				float highestScaleKey	 = channel.scalings.size()  ? channel.scalings [channel.scalings.size()  - 1].key : 0.f;

				ImGui::Text("Channel name: %s | Highest pos key: %2.f, rot key: %2.f, scale key: %2.f", channelName.c_str(), highestPositionKey, highestRotationKey, highestScaleKey);
			}
		}
	}
}

void AssetViewerUI::displayFontInfo(AssetInfo<Font>& descriptor) {
	static const unsigned int minVal = 1;
	static const unsigned int maxVal = 100;

	ImGui::SliderScalar("Font Size", ImGuiDataType_U32, &copyOfSelectedFontSize, &minVal, &maxVal, "%u");

	if (ImGui::IsItemDeactivatedAfterEdit()) {
		if (copyOfSelectedFontSize == descriptor.fontSize) {
			return;
		}

		descriptor.fontSize = copyOfSelectedFontSize;
		recompileResourceWithUpdatedDescriptor<Font>(descriptor);
	}
}

void AssetViewerUI::displayPrefabInfo([[maybe_unused]] AssetInfo<Prefab>& descriptor) {
	auto&& prefabRegistry = editor.engine.prefabManager.getPrefabRegistry();

	if (ImGui::TreeNode("Prefab Hierarchy")) {
		ImGui::BeginChild("##Prefab", ImVec2(0.f, 0.f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);
		
		editor.displayEntityHierarchy(prefabRegistry, rootPrefabEntity, true,
			[&](std::vector<entt::entity> entities) {
				if (entities.empty()) {
					return;
				}

				selectedPrefabEntity = entities[0];
			},
			[&](entt::entity entity) {
				return selectedPrefabEntity == entity;
			}
		);

		ImGui::EndChild();
		ImGui::TreePop();
	}

	ImGui::Separator();
	g_displayComponentFunctor(editor, selectedPrefabEntity, editor.engine.prefabManager.getPrefabRegistry());
}

void AssetViewerUI::displayBoneHierarchy(BoneIndex boneIndex, Skeleton const& skeleton) {
	Bone const& bone = skeleton.bones[boneIndex];

	bool isOpen = ImGui::TreeNodeEx(bone.name.c_str());

	if (isOpen) {
		for (auto& boneChildrenIndex : bone.boneChildrens) {
			displayBoneHierarchy(boneChildrenIndex, skeleton);
		}

		ImGui::TreePop();
	}
}

void AssetViewerUI::displayNodeHierarchy(ModelNodeIndex nodeIndex, Skeleton const& skeleton) {
	ModelNode const& node = skeleton.nodes[nodeIndex];

	bool isOpen = ImGui::TreeNodeEx(std::string{ (node.isBone ? ICON_FA_BONE : "") + node.name }.c_str());

	if (isOpen) {
		for (auto& nodeChildrenIndex : node.nodeChildrens) {
			displayNodeHierarchy(nodeChildrenIndex, skeleton);
		}

		ImGui::TreePop();
	}
}

#include "Editor/ComponentInspection/displayComponent.ipp"
