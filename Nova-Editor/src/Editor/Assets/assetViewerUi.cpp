#include <iterator> 
#include <regex>

#include "assetViewerUi.h"

#include "imgui.h"
#include "IconsFontAwesome6.h"

#include "misc/cpp/imgui_stdlib.h"

#include "magic_enum.hpp"

AssetViewerUI::AssetViewerUI(AssetManager& assetManager, ResourceManager& resourceManager) :
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

#if 0
	ImGui::InputText(selectedResourceExtension.empty() ? "##" : selectedResourceExtension.c_str(), &selectedResourceStemCopy);

	if (ImGui::IsItemDeactivatedAfterEdit() && std::filesystem::path{ descriptorPtr->filepath }.stem() != selectedResourceStemCopy) {
		if (!assetManager.renameFile(selectedResourceId, selectedResourceStemCopy)) {
			// reset rename attempt.
			selectedResourceStemCopy = std::filesystem::path{ descriptorPtr->filepath }.stem().string();
		}
	}
#endif

	auto displayResourceUIFunctor = [&]<ValidResource ...T>(ResourceID id) {
		([&] {
			if (resourceManager.isResource<T>(id)) {
				displayAssetUI<T>(*descriptorPtr);

				if (toSerialiseSelectedDescriptor) {
					if constexpr (std::same_as<T, ScriptAsset>) {
						updateScriptFileName(descriptorPtr->filepath, selectedResourceName, selectedResourceId);
					}

					assetManager.serialiseDescriptor<T>(selectedResourceId);
				}
			}
		}(), ...);
	};

	displayResourceUIFunctor.template operator()<ALL_RESOURCES>(selectedResourceId);
	ImGui::End();
#endif
}

void AssetViewerUI::updateScriptFileName(AssetFilePath const& filepath, std::string const& newName, [[maybe_unused]] ResourceID id) {
	std::ifstream inputScriptFile{ filepath };

	if (!inputScriptFile) {
		Logger::error("Fail to read script file.");
		return;
	}

	// we read the whole file's content into a variable
	std::string scriptContents{ std::istreambuf_iterator<char>(inputScriptFile), std::istreambuf_iterator<char>() };

	// i love regex. ask me if u need regex explaination.
	std::regex classRegex(R"(class [\w\s]+\s?:)");
	scriptContents = std::regex_replace(scriptContents, classRegex, "class " + newName + " :");
	
	std::ofstream outputScriptFile{ filepath };

	if (!outputScriptFile) {
		Logger::error("Fail to open script file for writing.");
		return;
	}

	outputScriptFile << scriptContents;

	Logger::info("Succesfully updated script name.");
}

void AssetViewerUI::selectNewResourceId(ResourceID id) {
	selectedResourceId = id;

	auto descriptorPtr = assetManager.getDescriptor(selectedResourceId);

	if (!descriptorPtr) {
		return;
	}

	selectedResourceName = descriptorPtr->name;
	selectedResourceStemCopy = std::filesystem::path{ descriptorPtr->filepath }.filename().string();
	selectedResourceExtension = std::filesystem::path{ descriptorPtr->filepath }.extension().string();
}

void AssetViewerUI::displayTextureInfo(AssetInfo<Texture>& textureInfo) {
	ImGui::SeparatorText("Texture");

	constexpr auto listOfEnumValues = magic_enum::enum_entries<AssetInfo<Texture>::Compression>();

	if (ImGui::BeginCombo("Compression", std::string{ magic_enum::enum_name<AssetInfo<Texture>::Compression>(textureInfo.compression) }.c_str())) {
		for (auto&& [enumValue, enumInString] : listOfEnumValues) {
			if (ImGui::Selectable(std::string{ enumInString }.c_str(), enumValue == textureInfo.compression)) {
				if (enumValue != textureInfo.compression) {
					textureInfo.compression = enumValue;

					// serialise immediately..
					assetManager.serialiseDescriptor<Texture>(selectedResourceId);

					// we make a copy of asset info, because the reference is getting invalidated..
					AssetInfo<Texture> textureInfoCopy = textureInfo;

					// we remove this old resource..
					resourceManager.removeResource(selectedResourceId);
					assetManager.removeResource(selectedResourceId);

					// recompile.., will add to resource manager if compilation is successful.
					assetManager.createResourceFile<Texture>(textureInfoCopy);
				}
			}
		}

		ImGui::EndCombo();
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

	if (ImGui::CollapsingHeader("Model")) {
		ImGui::SeparatorText(std::string{ "Sub meshes: " + std::to_string(model->meshes.size()) }.c_str());

		int counter = 0;

		for (auto& mesh : model->meshes) {
			ImGui::PushID(counter);

			ImGui::Text(mesh.name.c_str());
			ImGui::BeginChild("##", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Borders);

			ImGui::Text("Vertices: %zu", mesh.vertices.size());
			ImGui::Text("Indices: %zu", mesh.indices.size());
			ImGui::Text("Material name: %s", mesh.materialName.c_str());

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

void AssetViewerUI::displayAnimationInfo(AssetInfo<Model>& descriptor) {
	auto&& [animation, loadStatus] = resourceManager.getResource<Model>(selectedResourceId);

	if (!animation) {
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

	if (ImGui::CollapsingHeader("Animation")) {
		for (auto&& animation : animation->animations) {
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
