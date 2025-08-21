#include "Libraries/reflection.h"
#include "assetManager.h"

#include <glm/gtc/type_ptr.hpp>
#include <concepts>

void displayMaterialUI(Material& material, ComponentInspector& componentInspector);

namespace {
	// https://stackoverflow.com/questions/54182239/c-concepts-checking-for-template-instantiation
	template<class T>
	concept IsTypedAssetID = requires(T x) {
		{ TypedAssetID{ x } } -> std::same_as<T>;
	};

	template<typename Component>
	void displayComponent(ComponentInspector& componentInspector, entt::entity entity, Component& component) {
		(void) componentInspector;
		(void) entity;
		(void) component;

		if constexpr (!reflection::isReflectable<Component>()) {
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
			bool toDisplay = ImGui::CollapsingHeader(name);

			if (!toDisplay) {
				return;
			}

			ImGui::PushID(static_cast<int>(entity));
			ImGui::PushID(static_cast<int>(typeid(Component).hash_code()));

			// Visits each of the component's data member.
			reflection::visit([&](auto fieldData) {
				auto& dataMember = fieldData.get();
				constexpr const char* dataMemberName = fieldData.name();
				using DataMemberType = std::decay_t<decltype(dataMember)>;

				ImGui::PushID(static_cast<int>(std::hash<std::string>{}(dataMemberName)));

				// here we gooooooooooooooooooooooooooooooo! time to list down all the primitives!
				if constexpr (std::same_as<DataMemberType, int>) {
					ImGui::InputInt(fieldData.name(), &dataMember);
				}

				if constexpr (std::same_as<DataMemberType, float>) {
					ImGui::InputFloat(fieldData.name(), &dataMember);
				}

				if constexpr (std::same_as<DataMemberType, bool>) {
					ImGui::Checkbox(fieldData.name(), &dataMember);
				}

				if constexpr (std::same_as<DataMemberType, std::string>) {
					ImGui::PushTextWrapPos();
					ImGui::Text(fieldData.name());
					ImGui::InputText("##", &dataMember);
					ImGui::PopTextWrapPos();
				}

				if constexpr (std::same_as<DataMemberType, glm::vec3>) {
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

				if constexpr (std::same_as<DataMemberType, Color>) {
					ImGui::Text(fieldData.name());
					glm::vec3 vec = dataMember;
					ImGui::ColorEdit3("##", glm::value_ptr(vec));
					dataMember = vec;
				}

				if constexpr (std::same_as<DataMemberType, glm::quat>) {
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

				if constexpr (std::same_as<DataMemberType, EulerAngles>) {
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

				if constexpr (std::same_as<DataMemberType, AssetID>) {
					ImGui::Text(dataMemberName);

					AssetManager& assetManager = componentInspector.editor.engine.assetManager;
					Asset* asset = assetManager.getAssetInfo(dataMember);

					if (!asset) {
						ImGui::Text("This asset id [%zu] is invalid!", static_cast<std::size_t>(dataMember));
					}
					else {
						ImGui::Text("Asset ID: [%zu]", static_cast<std::size_t>(dataMember));
					}
				}

				if constexpr (IsTypedAssetID<DataMemberType>) {
					// dataMember is of type TypedAssetID<T>
					using OriginalAssetType = DataMemberType::AssetType;

					componentInspector.displayAssetDropDownList<OriginalAssetType>(dataMember, dataMemberName, [&](AssetID assetId) {
						dataMember = DataMemberType{ assetId };
					});
				}

				if constexpr (std::same_as<DataMemberType, std::unordered_map<MaterialName, Material>>) {
					int i = 0;
					for (auto&& [name, material] : dataMember) {
						ImGui::PushID(i);

						ImGui::Text(std::string{ "Material [" + std::to_string(i) + "]: " + name }.c_str());
						displayMaterialUI(material, componentInspector);
						++i;

						ImGui::PopID();
					}
				}

				ImGui::PopID();
			}, component);

			ImGui::PopID();
			ImGui::PopID();
		}
	}
}