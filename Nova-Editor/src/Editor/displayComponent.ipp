#include "reflection.h"
#include "ResourceManager/resourceManager.h"
#include "Audio/audioSystem.h"
#include "Engine/engine.h"

#include <glm/gtc/type_ptr.hpp>
#include <concepts>

#include "magic_enum.hpp"

void displayMaterialUI(Material& material, ComponentInspector& componentInspector);

namespace {
	// https://stackoverflow.com/questions/54182239/c-concepts-checking-for-template-instantiation
	template<class T>
	concept IsTypedResourceID = requires(T x) {
		{ TypedResourceID{ x } } -> std::same_as<T>;
	};

	template<typename Component>
	void displayComponent(ComponentInspector& componentInspector, entt::entity entity, Component& component) {
		(void) entity;

		ResourceManager& resourceManager = componentInspector.resourceManager;
		AudioSystem& audioSystem		 = componentInspector.audioSystem;
		(void) resourceManager;

		if constexpr (!reflection::isReflectable<Component>()) {
			return;
		}
		else if constexpr (std::same_as<Component, EntityData>) {
			return;
		}
		else {

#if defined(_MSC_VER)
			// let's hope msvc doesnt change implementation haha
			// removes the `struct ` infront of type name.

			// 2 local variable of string because of lifetime :c
			std::string originalTypeName = typeid(Component).name();
			std::string typeName = originalTypeName.substr(6);

			char const* name = typeName.c_str();
#else
			constexpr char const* name = typeid(Component).name();
#endif
			bool toDisplay;
			bool toShowHeader = true;

			// show the close button only for components that are not transform.
			if constexpr (!std::same_as<Component, Transform>) {
				toDisplay = ImGui::CollapsingHeader(name, &toShowHeader);
			}
			else {
				(void) toShowHeader;
				toDisplay = ImGui::CollapsingHeader(name);
			}

			if (!toDisplay) {
				// don't bother rendering each data member's ui if the collapsing header
				// is not active, go straight to end.
				goto end;
			}

			ImGui::PushID(static_cast<int>(entity));
			ImGui::PushID(static_cast<int>(typeid(Component).hash_code()));

			// Visits each of the component's data member and renders them.
			reflection::visit(
				[&](auto fieldData) {
					auto& dataMember = fieldData.get();
					constexpr const char* dataMemberName = fieldData.name();
					using DataMemberType = std::decay_t<decltype(dataMember)>;

					ImGui::PushID(static_cast<int>(std::hash<std::string>{}(dataMemberName)));

					// here we gooooooooooooooooooooooooooooooo! time to list down all the primitives!
					if constexpr (std::same_as<DataMemberType, int>) {
						ImGui::InputInt(fieldData.name(), &dataMember);
					}

					else if constexpr (std::same_as<DataMemberType, float>) {
						ImGui::InputFloat(fieldData.name(), &dataMember);
					}

					else if constexpr (std::same_as<DataMemberType, bool>) {
						ImGui::Checkbox(fieldData.name(), &dataMember);
					}

					else if constexpr (std::same_as<DataMemberType, std::string>) {
						ImGui::PushTextWrapPos();
						ImGui::Text(fieldData.name());
						ImGui::InputText("##", &dataMember);
						ImGui::PopTextWrapPos();
					}

					else if constexpr (std::same_as<DataMemberType, glm::vec3>) {
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

					else if constexpr (std::same_as<DataMemberType, Color>) {
						ImGui::Text(fieldData.name());
						glm::vec3 vec = dataMember;
						ImGui::ColorEdit3("##", glm::value_ptr(vec));
						dataMember = vec;
					}

					else if constexpr (std::same_as<DataMemberType, glm::quat>) {
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

					else if constexpr (std::same_as<DataMemberType, EulerAngles>) {
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

					if constexpr (std::same_as<DataMemberType, Radian>) {
						// convert from radian to degrees for display..
						float angle = static_cast<float>(dataMember);
						angle = toDegree(angle);
						if (ImGui::SliderFloat(fieldData.name(), &angle, 0.f, 180.f)) {
							dataMember = toRadian(angle);
						}
					}

					if constexpr (std::same_as<DataMemberType, ResourceID>) {
						ImGui::Text(dataMemberName);

						Asset* asset = resourceManager.getResourceInfo(dataMember);

						if (!asset) {
							ImGui::Text("This asset id [%zu] is invalid!", static_cast<std::size_t>(dataMember));
						}
						else {
							ImGui::Text("Asset ID: [%zu]", static_cast<std::size_t>(dataMember));
						}
					}

					if constexpr (IsTypedResourceID<DataMemberType>) {
						// dataMember is of type TypedAssetID<T>
						using OriginalAssetType = DataMemberType::AssetType;

						componentInspector.displayAssetDropDownList<OriginalAssetType>(dataMember, dataMemberName, [&](ResourceID resourceId) {
							dataMember = DataMemberType{ resourceId };
						});
					}

					else if constexpr (std::same_as<DataMemberType, std::unordered_map<MaterialName, Material>>) {
						int i = 0;
						for (auto&& [name, material] : dataMember) {
							ImGui::PushID(i);

							if (ImGui::TreeNode(std::string{ "Material [" + std::to_string(i) + "]: " + name }.c_str())) {
								displayMaterialUI(material, componentInspector);
								ImGui::TreePop();
							}

							++i;

							ImGui::PopID();
						}
					}
					else if constexpr (std::same_as<DataMemberType, std::vector<ScriptData>>) {
						std::vector<ScriptData>& scriptDatas{ dataMember };

						componentInspector.displayAssetDropDownList<ScriptAsset>(std::nullopt, "Add new script", [&](ResourceID resourceId) {
							scriptDatas.push_back(ScriptData{ resourceId });
						});

						// List of Scripts
						int i{};

						// Removal of Scripts
						ImGui::BeginChild("", ImVec2(0, 400), ImGuiChildFlags_Border);

						std::vector<ScriptData>::iterator it = std::remove_if(std::begin(scriptDatas), std::end(scriptDatas), [&](ScriptData& scriptData) {
							ImGui::PushID(i++);
							bool keepScript = true;

							auto&& [scriptAsset, _] = resourceManager.getResource<ScriptAsset>(scriptData.scriptId);
							assert(scriptAsset);

							if (ImGui::CollapsingHeader(scriptAsset->getClassName().c_str(), &keepScript)) {
								// To do display additional data
							}

							ImGui::PopID();
							return !keepScript;
						});

						ImGui::EndChild();

						if(it != std::end(scriptDatas))
							scriptDatas.erase(it);
					}

					else if constexpr (std::same_as<DataMemberType, entt::entity>) {
						ImGui::Text("%s: %u", dataMemberName, dataMember);
					}


					if constexpr (std::same_as<DataMemberType, std::unordered_map<std::string, AudioData>>)
					{
						auto& audioDatas = dataMember;

						// Add Audio
						componentInspector.displayAssetDropDownList<Audio>(std::nullopt, "Add Audio File", [&](ResourceID resourceId)
							{
								auto&& [audioAsset, _] = resourceManager.getResource<Audio>(resourceId);
								assert(audioAsset);

								// Store full AudioData directly in the component
								audioDatas.emplace(audioAsset->getClassName().c_str(), AudioData{ resourceId, 1.0f, false });
							});

						// List of Audio Files
						int i{};
						ImGui::BeginChild("", ImVec2(0, 200), ImGuiChildFlags_Border);

						for (auto it = audioDatas.begin(); it != audioDatas.end();) {
							ImGui::PushID(i++);

							bool keepAudioFile = true;

							AudioData& audioData = it->second;
							auto&& [audioAsset, _] = resourceManager.getResource<Audio>(audioData.AudioId);
							assert(audioAsset);

							if (ImGui::CollapsingHeader(audioAsset->getClassName().c_str(), &keepAudioFile)) {
								// Play Audio
								if (ImGui::Button("Play Audio")) {
									if (audioSystem.isBGM(audioData.AudioId)) {
										audioSystem.playBGM(audioData.AudioId, audioData.Volume);
									}
									else {
										audioSystem.playSFX(audioData.AudioId, 0.f, 0.f, 0.f, audioData.Volume);
									}
								}
								// Update Playback Volume
								if (ImGui::DragFloat("Adjust Volume", &audioData.Volume, 0.10f, 0.0f, 2.0f, "%.2f")) {
									audioSystem.AdjustVol(audioData.AudioId, audioData.Volume);
								}
								// Stop Audio from playing inside Editor
								if (ImGui::Button("Stop Audio")) {
									audioSystem.StopAudio(audioData.AudioId);
								}
							}

							ImGui::PopID();

							if (!keepAudioFile) {
								it = audioDatas.erase(it);
							}
							else {
								++it;
							}
						}

						ImGui::EndChild();
					}


					// it's an enum. let's display a dropdown box for this enum.
					// how? using enum reflection provided by "magic_enum.hpp" :D
					else if constexpr (std::is_enum_v<DataMemberType>) {
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

					ImGui::PopID();
				},
			component);

			ImGui::PopID();
			ImGui::PopID();

		end:
			// prompted to delete component.
			if (!toShowHeader) {
				componentInspector.ecs.registry.erase<Component>(entity);
			}
		}
	}
}