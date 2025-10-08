#include "reflection.h"

#include "magic_enum.hpp"
#include "PropertyDisplay/displayProperties.h"

#include <concepts>

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
		AssetManager& assetManager		 = componentInspector.assetManager;
		AudioSystem& audioSystem		 = componentInspector.audioSystem;
		PropertyReferences propertyReferences{ entity,
											 componentInspector,
											 componentInspector.resourceManager,
											 componentInspector.assetManager,
											 componentInspector.audioSystem,
											 componentInspector.editor.engine.scriptingAPIManager,
											 componentInspector.editor.engine,
											 componentInspector.editor, 
											 componentInspector.editor.engine.ecs};
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
					
					// Specializations
					if constexpr (IsTypedResourceID<DataMemberType>) {
						// dataMember is of type TypedAssetID<T>
						using OriginalAssetType = DataMemberType::AssetType;

						componentInspector.editor.displayAssetDropDownList<OriginalAssetType>(dataMember, dataMemberName, [&](ResourceID resourceId) {
							dataMember = DataMemberType{ resourceId };
						});
					}
					else if constexpr (std::is_enum_v<DataMemberType>) {
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
					else
					{
						// Generalization
						DisplayProperty<DataMemberType>(propertyReferences, dataMemberName, dataMember);
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