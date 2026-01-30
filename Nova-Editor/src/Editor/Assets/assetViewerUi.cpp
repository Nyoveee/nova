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

constexpr int MAXIMUM_RESOURCE_HISOTRY_ENTRIES = 50;

namespace {
	// We work with 3 containers
	// 1. The updated shader's uniforms
	// 2. The outdated material's uniforms
	// 3. Intermediary output container that will represent the updated uniforms this material will have.

	// We want to update the outdated material uniform if there is a new entry, like
	// a new identifier or changed type.
	// We want the material to preserve its original value if that entry did not change.
	void updateMaterialProperty(Material& material, std::vector<UniformData> const& shaderUniforms /* 1. */) {
		std::vector<UniformData> newShaderUniforms;												/* 3. */
		std::vector<UniformData>& oldMaterialUniforms = material.materialData.uniformDatas;	/* 2. */

		// For each uniform..
		for (auto&& [type, identifier, value] : shaderUniforms) {
			// Try to find the same identifier..
			auto iterator = std::find_if(
				oldMaterialUniforms.begin(),
				oldMaterialUniforms.end(),
				[&](auto&& uniformData) {
					return uniformData.identifier == identifier;
				}
			);

			// This entry did not change (same identifier and type).. so let's use the old value..
			if (iterator != oldMaterialUniforms.end() && iterator->glslType == type && iterator->value.index() == value.index()) {
				// Re-use the old value..
				newShaderUniforms.push_back(*iterator);
				continue;
			}

			// Entry has changed, let's add some default constructed value to it..
			auto valueIterator = allValidShaderTypes.find(type);

			if (valueIterator == allValidShaderTypes.end()) {
				Logger::warn("Shader uniform contains invalid GLSL type of {}! Ignored..", type);
				continue;
			}

			auto uniformValue = valueIterator->second;

			// no choice, i gotta hardcode..
			if (identifier == "UVTiling") {
				uniformValue = glm::vec2{ 1.0f, 1.0f };
			}

			newShaderUniforms.push_back(UniformData{
				type,
				identifier,
				uniformValue	// value
			});
		}

		// let's update our material's uniform with the new uniforms..
		oldMaterialUniforms = std::move(newShaderUniforms);
	};
}
AssetViewerUI::AssetViewerUI(Editor& editor, AssetManager& assetManager, ResourceManager& resourceManager) :
	editor							{ editor },
	assetManager					{ assetManager },
	resourceManager					{ resourceManager },
	selectedResourceId				{ INVALID_RESOURCE_ID },
	toSerialiseSelectedDescriptor	{ false },
	toOverrideEditSystemResource	{ false },
	recentlyChangedResource			{ false }
{}

void AssetViewerUI::update() {
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

	// Display navigation history..
	displayNavigationHistory();

	toSerialiseSelectedDescriptor = false;

	// Display common shared asset info across all assets..
	ImGui::Text("Resource ID: %zu", static_cast<std::size_t>(descriptorPtr->id));

	// we store this in a variable because selected resource id may change midst execution of this window,
	bool isSystem = isSystemResource(selectedResourceId);
	bool toOverrideSystemDisable = toOverrideEditSystemResource;

	if (isSystem) {
		ImGui::TextWrapped("This is a system resource. In built into the engine, it is not meant to be modified.");
		ImGui::Checkbox("[Debug] Override disable.", &toOverrideEditSystemResource);
		
		if(!toOverrideSystemDisable) ImGui::BeginDisabled();
	}

	// ===== Display asset name ======
	ImGui::InputText(selectedResourceExtension.c_str(), &selectedResourceName);

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

	bool hasNamedChanged = ImGui::IsItemDeactivatedAfterEdit() && std::filesystem::path{ descriptorPtr->filepath }.stem() != selectedResourceName;

	auto displayResourceUIFunctor = [&]<ValidResource ...T>(ResourceID id) {
		([&] {
			if (resourceManager.isResource<T>(id)) {
				displayAssetUI<T>(*descriptorPtr);

				// If there is a name change, we serialise the asset, change it's name and filepath, then serialise the descriptor
				if (hasNamedChanged) {
					// serialise the asset.. (some assets are modified by the editor, so we need to serialise it)
					assetManager.serialiseResource<T>(id);

					if (!assetManager.renameFile(selectedResourceId, selectedResourceName)) {
						// reset rename attempt.
						selectedResourceName = std::filesystem::path{ descriptorPtr->filepath }.stem().string();
					}

					// if this resource is a script, we want to update the class name if possible..
					if constexpr (std::same_as<T, ScriptAsset>) {
						updateScriptFilePath(descriptorPtr->filepath, selectedResourceId);
					}

					assetManager.serialiseDescriptor(selectedResourceId);
				}
			}
		}(), ...);
	};

	displayResourceUIFunctor.template operator()<ALL_RESOURCES>(selectedResourceId);

	if (isSystem && !toOverrideSystemDisable) {
		ImGui::EndDisabled();
	}

	ImGui::End();

	handleRecompilation();
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
	if (id == selectedResourceId) {
		return;
	}

	toOverrideEditSystemResource = false;

	// populate history..
	if (selectedResourceId != INVALID_RESOURCE_ID) {
		previousResourceIds.push_back(selectedResourceId);
		nextResourceIds.clear();
	}

	selectResourceID(id);

	if (previousResourceIds.size() > MAXIMUM_RESOURCE_HISOTRY_ENTRIES) {
		previousResourceIds.pop_front();
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

	editor.displayAssetDropDownList<CustomShader>(shaderId, "Shader", [&](ResourceID id) {
		material->materialData.selectedShader = TypedResourceID<CustomShader>{ id };

		// Since we change shader, we need to reupdate the overridenUniforms..
		auto&& [customShader, __] = resourceManager.getResource<CustomShader>(id);

		if (!customShader) {
			assert(false && "The display asset drop down list should never provide an invalid resource id.");
			Logger::error("Selected an invalid shader!");
			return;
		}

		updateMaterialProperty(*material, customShader->customShaderData.uniformDatas);
	});
	
	if (!customShader) {
		return;
	}

	// first load..
	if (recentlyChangedResource) {
		recentlyChangedResource = false;
		updateMaterialProperty(*material, customShader->customShaderData.uniformDatas);
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
	
	ImGui::SeparatorText("Config");

	editor.displayEnumDropDownList<BlendingConfig>(material->materialData.blendingConfig, "Blending", [&](auto config) {
		material->materialData.blendingConfig = config;
	});

	editor.displayEnumDropDownList<DepthTestingMethod>(material->materialData.depthTestingMethod, "Depth Testing", [&](auto config) {
		material->materialData.depthTestingMethod = config;
	});

	editor.displayEnumDropDownList<CullingConfig>(material->materialData.cullingConfig, "Culling", [&](auto config) {
		material->materialData.cullingConfig = config;
	});

	ImGui::SeparatorText("Properties");

	int imguiCounter = 0;

	// Display the current material values..
	for (auto&& [type, name, uniformData] : material->materialData.uniformDatas) {
		ImGui::PushID(imguiCounter++);

		std::visit([&](auto&& value) {
			using Type = std::decay_t<decltype(value)>;

			if constexpr (std::same_as<Type, glm::mat3> || std::same_as<Type, glm::mat4>) {
				assert(type == "mat3" || type == "mat4");
			}
			else {
				DisplayProperty<Type>(editor, name.c_str(), value);
			}
		}, uniformData);

		ImGui::PopID();
	}

	ImGui::SeparatorText("Debug");
	
	if (ImGui::Button("Update Material Properties")) {
		updateMaterialProperty(*material, customShader->customShaderData.uniformDatas);
	}

	// this updates EVERY SINGLE MATERIAL in the engine.
	// this is important if the shaders system got reworked, for an example, or if there is a new entry, etc..
	if (ImGui::Button("MASS UPDATE ALL MATERIALS.")) {
		for (ResourceID id : resourceManager.getAllResources<Material>()) {
			auto&& [mat, __] = resourceManager.getResource<Material>(id);

			if (!mat) {
				continue;
			}

			// Chosen Shader
			TypedResourceID<CustomShader> const& customShaderId = mat->materialData.selectedShader;
			auto&& [shader, ___] = resourceManager.getResource<CustomShader>(customShaderId);

			if (!shader) {
				continue;
			}

			updateMaterialProperty(*mat, shader->customShaderData.uniformDatas);
		}
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
			if (ImGui::Button("Print")) {
				std::cout << openglShader.getVertexShader() << '\n';
			}

			ImGui::BeginChild("Vertex Shader", ImVec2{}, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY);
			ImGui::Text("%s", openglShader.getVertexShader().c_str());
			ImGui::EndChild();

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Fragment Shader")) {
			if (ImGui::Button("Print")) {
				std::cout << openglShader.getFragmentShader() << '\n';
			}

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

	ImGui::Separator();

	auto&& [texture, loadStatus] = resourceManager.getResource<Texture>(selectedResourceId);

	if (!texture) {
		switch (loadStatus)
		{
		case ResourceManager::QueryResult::Invalid:
			ImGui::Text("Resource ID is invalid.");
			return;
		case ResourceManager::QueryResult::WrongType:
			ImGui::Text("This should never happened. Resource ID is not a texture?");
			assert(false && "Resource ID is not a texture.");
			return;
		case ResourceManager::QueryResult::Loading:
			ImGui::Text("Loading..");
			return;
		case ResourceManager::QueryResult::LoadingFailed:
			ImGui::Text("Loading of texture failed.");
			return;
		default:
			assert(false);
			return;
		}
	}

	ImGui::Text("Width: %d", texture->getWidth());
	ImGui::Text("Height: %d", texture->getHeight());

	ImGui::Image(texture->getTextureId(), ImVec2{ static_cast<float>(texture->getWidth()), static_cast<float>(texture->getHeight()) });
}

void AssetViewerUI::displayModelInfo(AssetInfo<Model>& descriptor) {
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

	if (ImGui::CollapsingHeader("Model")) {
		ImGui::Text("Max dimension: %.2f", model->maxDimension);

		ImGui::BeginDisabled();
		DisplayProperty<glm::vec3>(editor, "Max Bound", model->maxBound);
		DisplayProperty<glm::vec3>(editor, "Min Bound", model->minBound);
		DisplayProperty<glm::vec3>(editor, "Center", model->center);
		DisplayProperty<glm::vec3>(editor, "Extents", model->extents);
		ImGui::EndDisabled();

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
	}

	if (model->skeleton) {
		if (ImGui::CollapsingHeader("Skeleton")) {
			Skeleton& skeleton = model->skeleton.value();

			// Display socket details..
			ImGui::SeparatorText("Sockets");

			if (skeleton.sockets.empty()) {
				ImGui::Text("No sockets.");
			}
			else {
				for (int i = 0; i < skeleton.sockets.size(); ++i) {
					BoneIndex socket = skeleton.sockets[i];
					std::string const& boneName = skeleton.bones[socket].name;
					ImGui::Text(std::string{ "Socket " + std::to_string(i) + " : " + boneName }.c_str());
				}
			}

			ImGui::SeparatorText(std::string{ "Bones: " + std::to_string(skeleton.bones.size()) }.c_str());
			
			displayBoneHierarchy(descriptor, skeleton.rootBone, skeleton);

			ImGui::SeparatorText(std::string{ "[Debug] Nodes: " + std::to_string(skeleton.nodes.size()) }.c_str());
			displayNodeHierarchy(skeleton.rootNode, skeleton);
		}
	}

	displayAnimationInfo(*model);
}

void AssetViewerUI::displayAnimationInfo(Model const& animationResource) {
	if (ImGui::CollapsingHeader("Animation")) {
		for (auto&& animation : animationResource.animations) {
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

	if (ImGui::Button("BroadCast")) {
		editor.engine.prefabManager.prefabBroadcast(selectedResourceId);
	}

	if (ImGui::TreeNodeEx("Prefab Hierarchy", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::BeginChild("##Prefab", ImVec2(0.f, 0.f), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);
		
		editor.displayEntityHierarchy(prefabRegistry, rootPrefabEntity, true, "PREFAB_HIERARCHY_ITEM",
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
	
	// Display entity data..
	EntityData* prefabEntityData = prefabRegistry.try_get<EntityData>(selectedPrefabEntity);

	if (!prefabEntityData) {
		ImGui::Text("Invalid entity!");
		return;
	}

	if (ImGui::CollapsingHeader("Prefab Data")) {
		ImGui::InputText("Name", &prefabEntityData->name);

		ImGui::Text("Prefab Entity ID: %u", static_cast<unsigned>(selectedPrefabEntity));
		ImGui::Text("Parent: %s", prefabEntityData->parent == entt::null ? "None" : prefabRegistry.get<EntityData>(prefabEntityData->parent).name.c_str());

		ImGui::Text("Direct children: ");

		ImGui::BeginChild("Direct children", ImVec2{ 0.f, 70.f }, ImGuiChildFlags_Borders);
		for (entt::entity child : prefabEntityData->children) {
			ImGui::BulletText(prefabRegistry.get<EntityData>(child).name.c_str());
		}

		ImGui::EndChild();

		ImGui::Text("Prefab ID: %zu", static_cast<std::size_t>(prefabEntityData->prefabID));
	}

	editor.displayingPrefabScripts = true;
	g_displayComponentFunctor(editor, selectedPrefabEntity, editor.engine.prefabManager.getPrefabRegistry(), false);
	editor.displayingPrefabScripts = false;

	editor.componentInspector.displayComponentDropDownList<ALL_COMPONENTS>(selectedPrefabEntity, editor.engine.prefabManager.getPrefabRegistry());
}

void AssetViewerUI::displayBoneHierarchy(AssetInfo<Model>& descriptor, BoneIndex boneIndex, Skeleton& skeleton) {
	Bone const& bone = skeleton.bones[boneIndex];

	bool isOpen = ImGui::TreeNodeEx(bone.name.c_str(), ImGuiTreeNodeFlags_AllowItemOverlap);

	auto socket = std::find(skeleton.sockets.begin(), skeleton.sockets.end(), boneIndex);
	bool isSocket = socket != skeleton.sockets.end();

	ImGui::SameLine();
	float checkboxPadding = ImGui::GetTextLineHeightWithSpacing() * 0.125f;
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ checkboxPadding, checkboxPadding });

	if (ImGui::Checkbox(("Bone " + std::to_string(boneIndex)).c_str(), &isSocket)) {
		if (isSocket) {
			skeleton.sockets.push_back(boneIndex);
		}
		else {
			skeleton.sockets.erase(socket);
		}

		descriptor.sockets = skeleton.sockets;
		recompileResourceWithUpdatedDescriptor<Model>(descriptor);
	}

	ImGui::PopStyleVar();

	if (isOpen) {
		for (auto& boneChildrenIndex : bone.boneChildrens) {
			displayBoneHierarchy(descriptor, boneChildrenIndex, skeleton);
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

void AssetViewerUI::selectResourceID(ResourceID id) {
	selectedResourceId = id;

	auto descriptorPtr = assetManager.getDescriptor(selectedResourceId);

	if (!descriptorPtr) {
		return;
	}

	// save a copy of resource name..
	selectedResourceName = descriptorPtr->name;
	selectedResourceExtension = std::filesystem::path{ descriptorPtr->filepath }.extension().string();

	// save a copy of the current font size..
	if (resourceManager.isResource<Font>(id)) {
		AssetInfo<Font>* fontDescriptorPtr = static_cast<AssetInfo<Font>*>(descriptorPtr);
		copyOfSelectedFontSize = fontDescriptorPtr->fontSize;
	}
	// save a copy of the current scale
	else if (resourceManager.isResource<Model>(id)) {
		AssetInfo<Model>* modelDescriptorPtr = static_cast<AssetInfo<Model>*>(descriptorPtr);
		copyOfScale = modelDescriptorPtr->scale;
	}
	// we need to make sure that this prefab is loaded before displaying the prefav details.
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

	recentlyChangedResource = true;
}

void AssetViewerUI::selectPreviousResourceID() {
	if (previousResourceIds.empty()) {
		return;
	}

	// pops the most recent resource id..
	ResourceID newResourceId = previousResourceIds.back();
	previousResourceIds.pop_back();

	// update next resource ids..
	if (selectedResourceId != INVALID_RESOURCE_ID) {
		nextResourceIds.push_back(selectedResourceId);

		if (nextResourceIds.size() > MAXIMUM_RESOURCE_HISOTRY_ENTRIES) {
			nextResourceIds.pop_front();
		}
	}

	selectResourceID(newResourceId);
}

void AssetViewerUI::selectNextResourceID() {
	if (nextResourceIds.empty()) {
		return;
	}

	// pops the most recent resource id..
	ResourceID newResourceId = nextResourceIds.back();
	nextResourceIds.pop_back();

	// update previous resource ids..
	if (selectedResourceId != INVALID_RESOURCE_ID) {
		previousResourceIds.push_back(selectedResourceId);

		if (previousResourceIds.size() > MAXIMUM_RESOURCE_HISOTRY_ENTRIES) {
			previousResourceIds.pop_front();
		}
	}

	selectResourceID(newResourceId);
}

void AssetViewerUI::displayNavigationHistory() {
	constexpr ImVec2 buttonSize = { 30.f, 30.f };

	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 0));

	if (!ImGui::BeginTable("Navigation Tab", 2)) {
		ImGui::PopStyleVar();
		return;
	}

	ImGui::BeginDisabled(previousResourceIds.empty());

	ImGui::TableNextRow();
	ImGui::TableNextColumn();

	if (ImGui::Button(ICON_FA_ARROW_LEFT, buttonSize)) {
		selectPreviousResourceID();
	}

	if (previousResourceIds.size()) {
		if (std::string const* name = assetManager.getName(previousResourceIds.back())) {
			ImGui::SameLine();
			ImGui::Text("%s", name->c_str());
		}
	}

	ImGui::EndDisabled();
	ImGui::TableNextColumn();
	
	ImGui::BeginDisabled(nextResourceIds.empty());

	if (nextResourceIds.size()) {
		if (std::string const* name = assetManager.getName(nextResourceIds.back())) {
			float textWidth = ImGui::CalcTextSize(name->c_str()).x;

			// right align.
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - textWidth - buttonSize.x - 10.f);

			ImGui::Text("%s", name->c_str());
			ImGui::SameLine();
		}
	}

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetColumnWidth() - buttonSize.x);

	if (ImGui::Button(ICON_FA_ARROW_RIGHT, buttonSize)) {
		selectNextResourceID();
	}

	ImGui::EndDisabled();
	ImGui::EndTable();

	ImGui::PopStyleVar();
}

void AssetViewerUI::handleRecompilation() {
	if (recompileAssetWithDescriptor) {
		recompileAssetWithDescriptor();
		recompileAssetWithDescriptor = nullptr;
	}
}

#include "Editor/ComponentInspection/displayComponent.ipp"
