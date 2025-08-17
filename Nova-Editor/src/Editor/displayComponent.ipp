#include "Libraries/reflection.h"
#include "engine.h"
#include "assetManager.h"

#include <concepts>

namespace {
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

				ImGui::PopID();
			}, component);

			ImGui::PopID();
		}
	}
}