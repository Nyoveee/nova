template <typename T> // T can be either MeshRenderer or SkinnedMeshRenderer.
void displayRendererComponent(Editor& editor, T& rendererComponent, entt::entity entity) {
	// ========================================
	// Display the model asset drop down..
	// ========================================

	editor.displayAssetDropDownList<Model>(rendererComponent.modelId, "Model", [&](ResourceID newModelId) {
		// changing model requires updating the renderer component's material vector.
		rendererComponent.modelId = TypedResourceID<Model>{ newModelId };

		// get their component..
		T* rendererComponent = editor.engine.ecs.registry.try_get<T>(entity);

		if (!rendererComponent) {
			return;
		}

		auto&& [model, _] = editor.resourceManager.getResource<Model>(newModelId);

		// invalid model..
		if (!model) {
			return;
		}

		std::vector<TypedResourceID<Material>> materialIds{};
		materialIds.resize(model->materialNames.size(), TypedResourceID<Material>{ DEFAULT_PBR_MATERIAL_ID });
		rendererComponent->materialIds = materialIds;
	});

	// ========================================
	// Display all the materials needed..
	// ========================================
	ImGui::SeparatorText("Material");

	ImGui::TextWrapped("If the model has only 1 sub mesh, you can attach multiple materials for additional render passes. Else, the rest will be ignored.");

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
		auto&& [model, _] = editor.resourceManager.getResource<Model>(rendererComponent.modelId);

		if (!model || !model->skeleton) {
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

		// display it..
		for (auto&& [boneId, attachedEntity] : rendererComponent.socketConnections) {
			Bone const& bone = model->skeleton->bones[boneId];

			editor.displayAllEntitiesDropDownList(bone.name.c_str(), attachedEntity, [&](entt::entity selectedEntity) {
				EntityData* entityData = editor.engine.ecs.registry.try_get<EntityData>(attachedEntity);
				if (entityData) entityData->attachedSocket = NO_BONE;

				attachedEntity = selectedEntity;

				// 
				entityData = editor.engine.ecs.registry.try_get<EntityData>(attachedEntity);
				if(entityData) entityData->attachedSocket = boneId;
			});
		}
	}

}