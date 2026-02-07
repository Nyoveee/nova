template <typename T> // T can be either MeshRenderer or SkinnedMeshRenderer.
void displayRendererComponent(Editor& editor, T& rendererComponent, entt::entity entity) {
	// ========================================
	// Display the model asset drop down..
	// ========================================
	auto&& [model, _] = editor.resourceManager.getResource<Model>(rendererComponent.modelId);

	editor.displayAssetDropDownList<Model>(rendererComponent.modelId, "Model", [&](ResourceID newModelId) {
		// changing model requires updating the renderer component's material vector.
		rendererComponent.modelId = TypedResourceID<Model>{ newModelId };

		auto&& [model, _] = editor.resourceManager.getResource<Model>(newModelId);

		// invalid model..
		if (!model) {
			return;
		}

		auto const* descriptor = editor.assetManager.getDescriptor(rendererComponent.modelId);
		AssetInfo<Model> const* typedDescriptor = dynamic_cast<AssetInfo<Model> const*>(descriptor);
		std::vector<TypedResourceID<Material>> materialIds{};

		if (!typedDescriptor) {
			materialIds.resize(model->materialNames.size(), TypedResourceID<Material>{ DEFAULT_PBR_MATERIAL_ID });
		}
		else {
			materialIds = typedDescriptor->materials;
		}

		rendererComponent.materialIds = std::move(materialIds);
	});

	if (!model) {
		ImGui::Text("Invalid model.");
		return;
	}

	// ========================================
	// Display all the materials needed..
	// ========================================
	ImGui::SeparatorText("Material");

	if (model->materialNames.size() == 1) {
		ImGui::TextWrapped("This model only requires 1 material. You can add more material to specify multiple render passes.");
	}
	else {
		ImGui::TextWrapped("This model has multiple submesh, with each submesh having their own material specification.");
	}

	for (unsigned int i = 0; i < rendererComponent.materialIds.size(); ++i) {
		TypedResourceID<Material>& id = rendererComponent.materialIds[i];

		std::string label = "[" + std::to_string(i) + "]";
		editor.displayAssetDropDownList<Material>(id, label.c_str(), [&](ResourceID resourceId) {
			id = TypedResourceID<Material>{ resourceId };
		});
	}

	if (ImGui::Button("[+]")) {
		rendererComponent.materialIds.push_back(TypedResourceID<Material>{ DEFAULT_PBR_MATERIAL_ID });
	}

	// ========================================
	// Display all the sockets needed..
	// ========================================
	if constexpr (std::same_as<T, SkinnedMeshRenderer>) {
		if (!model->skeleton) {
			return;
		}

		// ensure validity of skinned mesh renderer's socket..
		std::unordered_map<BoneIndex, entt::entity> sockets;

		for (auto&& socket : model->skeleton->sockets) {
			auto iterator = rendererComponent.socketConnections.find(socket);

			if (iterator == rendererComponent.socketConnections.end()) {
				sockets.insert({ socket, entt::null });
			}
			else {
				sockets.insert({ socket, iterator->second });
			}
		}

		rendererComponent.socketConnections = std::move(sockets);

		ImGui::SeparatorText("Bone Sockets");

		EntityData const& entityData = editor.engine.ecs.registry.get<EntityData>(entity);

		// we keep track of a new entry.. iterate through the renderer component and remove old entry..
		BoneIndex newBoneIndex = NO_BONE; 
		entt::entity newAttachedEntity = entt::null;

		// display all children..
		for (auto&& [boneId, attachedEntity] : rendererComponent.socketConnections) {
			Bone const& bone = model->skeleton->bones[boneId];

			editor.displayAllEntitiesDropDownList(bone.name.c_str(), attachedEntity, entityData.children, [&](entt::entity selectedEntity) {
				if (attachedEntity == selectedEntity) {
					return;
				}

				newBoneIndex = boneId;
				newAttachedEntity = selectedEntity;

				// detach..
				EntityData* attachedEntityData = editor.engine.ecs.registry.try_get<EntityData>(attachedEntity);
				if (attachedEntityData) {
					attachedEntityData->attachedSocket = NO_BONE;
				}

				attachedEntity = selectedEntity;

				// attach..
				attachedEntityData = editor.engine.ecs.registry.try_get<EntityData>(attachedEntity);
				if (attachedEntityData) attachedEntityData->attachedSocket = boneId;
			});
		}
		
		// remember to detach the newly selected entity from the other sockets..
		if (newAttachedEntity != entt::null) {
			for (auto&& [boneId, attachedEntity] : rendererComponent.socketConnections) {
				if (boneId == newBoneIndex) {
					continue;
				}

				if (attachedEntity != newAttachedEntity) {
					continue;
				}

				attachedEntity = entt::null;
			}
		}
	}

	// Display others..
	ImGui::Checkbox("Casts Shadow?", &rendererComponent.castShadow);
	ImGui::Checkbox("Cull front face for shadow pass?", &rendererComponent.shadowCullFrontFace);
}