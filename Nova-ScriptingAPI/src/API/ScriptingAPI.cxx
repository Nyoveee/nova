#include "ScriptingAPI.hxx"
#include "ScriptLibrary/Core/Script.hxx"
#include "ScriptLibrary/Extensions/ScriptLibrary.hxx"
#include "IManagedComponent.hxx"
#include "IManagedStruct.h"
#include "ResourceManager/resourceManager.h"

#include <sstream>
#include <filesystem>
#include <msclr/marshal_cppstd.h>



#include "scriptAsset.h"

generic<typename T> where T : Script
T Interface::tryGetScriptReference(System::UInt32 entityID)
{
	
	// Go through the managed scripts
	if (gameObjectScripts->ContainsKey(entityID)) {
		for each (System::UInt64 scriptID in gameObjectScripts[entityID]->Keys) {
			System::Type^ inheritedType{ gameObjectScripts[entityID][scriptID]->GetType() };
			do {
				if (inheritedType == T::typeid)
					return safe_cast<T>(gameObjectScripts[entityID][scriptID]); // Casting from one reference type to another
				inheritedType = inheritedType->BaseType;
			} while (inheritedType != nullptr);
		}
	}
	if (createdGameObjectScripts->ContainsKey(entityID)) {
		// not in managed scripts, perhaps in the created object script dictionary..
		for each (System::UInt64 scriptID in createdGameObjectScripts[entityID]->Keys) {
			System::Type^ inheritedType{ createdGameObjectScripts[entityID][scriptID]->GetType() };
			do {
				if (inheritedType == T::typeid)
					return safe_cast<T>(createdGameObjectScripts[entityID][scriptID]); // Casting from one reference type to another
				inheritedType = inheritedType->BaseType;
			} while (inheritedType != nullptr);
		}
	}
	

	return T();
}

void Interface::init(Engine& p_engine, const char* p_runtimePath)
{
	// Get the reference to the engine
	engine = &p_engine;
	runtimePath = p_runtimePath;
	// Instantiate the containers
	gameObjectScripts = gcnew System::Collections::Generic::Dictionary<System::UInt32, System::Collections::Generic::Dictionary<System::UInt64,Script^>^>();
	availableScripts = gcnew System::Collections::Generic::Dictionary<ScriptID, Script^>();
	createdGameObjectScripts = gcnew System::Collections::Generic::Dictionary<System::UInt32, System::Collections::Generic::Dictionary<System::UInt64, Script^>^>();
	timeoutDelegates = gcnew System::Collections::Generic::List<TimeoutDelegate^>();
	executeTimeoutDelegates = gcnew System::Collections::Generic::List<TimeoutDelegate^>();
	abstractScriptTypes = gcnew System::Collections::Generic::Dictionary<ScriptID, System::Type^>();

	assemblyLoadContext = nullptr;
}

void Interface::intializeAllScripts()
{
	// Instantiate all scripts..
	for each (System::UInt32 entityID in gameObjectScripts->Keys)
		for each (System::UInt64 scriptID in gameObjectScripts[entityID]->Keys)
			gameObjectScripts[entityID][scriptID]->callInit();
}

void Interface::handleOnCollision(EntityID entityOne, EntityID entityTwo) {
	if (gameObjectScripts->ContainsKey(entityOne)) {
		for each (System::UInt64 scriptID in gameObjectScripts[entityOne]->Keys) {
			gameObjectScripts[entityOne][scriptID]->callOnCollisionEnter(entityTwo);
		}
	}

	if (gameObjectScripts->ContainsKey(entityTwo)) {
		for each (System::UInt64 scriptID in gameObjectScripts[entityTwo]->Keys) {
			gameObjectScripts[entityTwo][scriptID]->callOnCollisionEnter(entityOne);
		}
	}
}

void Interface::executeEntityScriptFunction(EntityID entityID, ScriptID scriptId, std::string const& name) {
	System::String^ functionName = msclr::interop::marshal_as<System::String^>(name);

	if (!gameObjectScripts->ContainsKey(entityID)) {
		Logger::warn("Attempt to invoke script function of unknown entity {}", entityID);
		return;
	}

	if (!gameObjectScripts[entityID]->ContainsKey(scriptId)) {
		Logger::warn("Attempt to invoke unknown script id {} of entity {}", scriptId, entityID);
		return;
	}

	Script^ script = gameObjectScripts[entityID][scriptId];
	System::Type^ scriptType = script->GetType();

	try {
		System::Reflection::MethodInfo^ function = scriptType->GetMethod(functionName);
		function->Invoke(script, nullptr);
	}
	catch (System::Exception^ ex) {
		Logger::warn("Error when invoking function name {} of script id {} of entity {}", msclr::interop::marshal_as<std::string>(ex->ToString()), scriptId, entityID);
	}
}

void Interface::submitGameObjectDeleteRequest(EntityID entityToBeDeleted) {
	deleteGameObjectQueue.Enqueue(entityToBeDeleted);
}

std::vector<FieldData> Interface::getScriptFieldDatas(ScriptID scriptID)
{
	using BindingFlags = System::Reflection::BindingFlags;
	std::vector<FieldData> fieldDatas{};
	
	if (!availableScripts->ContainsKey(scriptID)) {
		Logger::error("Failed to obtain script field data from script {}!", scriptID);
		return {};
	}

	Script^ script = availableScripts[scriptID];
	System::Type^ type = script->GetType();
	do {
		array<System::Reflection::FieldInfo^>^ fieldInfos = type->GetFields(BindingFlags::DeclaredOnly | BindingFlags::Instance | BindingFlags::Public | BindingFlags::NonPublic);
		for (int i = 0; i < fieldInfos->Length; ++i) {
			// Ignore the base class
			if (fieldInfos[i]->DeclaringType == Script::typeid)
				continue;
			System::Type^ fieldType = fieldInfos[i]->GetModifiedFieldType()->UnderlyingSystemType;
			FieldData field{};

			// Private and Protected members will only be added if they have the serializablefield attribute
			if (!fieldInfos[i]->IsPublic && fieldInfos[i]->GetCustomAttributes(SerializableField::typeid, true)->Length == 0)
				continue;

			field.name = msclr::interop::marshal_as<std::string>(fieldInfos[i]->Name);
			// GameObject
			if (fieldType == GameObject::typeid) {
				GameObject^ gameObject = safe_cast<GameObject^>(fieldInfos[i]->GetValue(script));
				field.data = entt::entity(gameObject ? gameObject->entityID : entt::null);
				fieldDatas.push_back(field);
				continue;
			}
			// Struct
			if (IManagedStruct^ managedStruct = dynamic_cast<IManagedStruct^>(fieldInfos[i]->GetValue(script))) {
				managedStruct->AppendValueToFieldData(field);
				fieldDatas.push_back(field);
				continue;
			}
			// Component
			if (fieldType->IsSubclassOf(IManagedComponent::typeid)) {
				IManagedComponent^ managedComponent = safe_cast<IManagedComponent^>(fieldInfos[i]->GetValue(script));
				field.data = entt::entity(managedComponent ? managedComponent->entityID : entt::null);
				fieldDatas.push_back(field);
				continue;
			}
			// Scripts
			if (fieldType->IsSubclassOf(Script::typeid)) {
				Script^ managedScripts = safe_cast<Script^>(fieldInfos[i]->GetValue(script));
				field.data = entt::entity(managedScripts ? managedScripts->entityID : entt::null);
				fieldDatas.push_back(field);
				continue;
			}

			// Typed Resource ID
			if (fieldType->IsSubclassOf(IManagedResourceID::typeid)) {
				if (ObtainTypedResourceIDFromScript<ALL_MANAGED_TYPED_RESOURCE_ID>(field, fieldInfos[i]->GetValue(script), fieldType)) {
					fieldDatas.push_back(field);
					continue;
				}
				else {
					Logger::warn("Typed resource type in script currently not supported for script serialization.");
					continue;
				}
			}
			// Strings
			if (fieldType->Equals(System::String::typeid)) {
				System::String^ string = safe_cast<System::String^>(fieldInfos[i]->GetValue(script));
				field.data = string ? Convert(string): "";
				fieldDatas.push_back(field);
				continue;
			}
			// Primitives
			if (fieldType->IsPrimitive && ObtainPrimitiveDataFromScript<ALL_FIELD_PRIMITIVES>(field, fieldInfos[i]->GetValue(script))) {
				fieldDatas.push_back(field);
				continue;
			}
			if (fieldType->IsPrimitive) {
				Logger::warn("Primitive type in script currently not supported for script serialization {}",
					msclr::interop::marshal_as<std::string>(fieldType->ToString()));
			}
		}
		type = type->BaseType;
	} while (type != nullptr);
	return fieldDatas;
}

void Interface::addEntityScript(EntityID entityID, ScriptID scriptId)
{
	if (!availableScripts->ContainsKey(scriptId)) {
		Logger::error("Failed to add invalid script {} for entity {}!", scriptId, entityID);
		return;
	}

	if (!gameObjectScripts->ContainsKey(entityID))
		gameObjectScripts[entityID] = gcnew ScriptDictionary();

	Script^ newScript = safe_cast<Script^>(System::Activator::CreateInstance(availableScripts[scriptId]->GetType()));

	newScript->entityID = entityID;
	// Set GameObject Details
	newScript->_gameObject = GameObject::GetReference(entityID);

	gameObjectScripts[entityID][scriptId] = newScript;
}

Script^ Interface::delayedAddEntityScript(EntityID entityID, ScriptID scriptId) {
	if (!availableScripts->ContainsKey(scriptId)) {
		Logger::error("Failed to add invalid script {} for entity {}!", scriptId, entityID);
		return nullptr;
	}

	if (!createdGameObjectScripts->ContainsKey(entityID))
		createdGameObjectScripts[entityID] = gcnew ScriptDictionary();

	Script^ newScript = safe_cast<Script^>(System::Activator::CreateInstance(availableScripts[scriptId]->GetType()));
	newScript->entityID = entityID;
	// Set GameObject Details
	newScript->_gameObject = GameObject::GetReference(entityID);
	createdGameObjectScripts[entityID][scriptId] = newScript;

	return newScript;
}

void Interface::initializeScript(EntityID entityID, ScriptID scriptId) {
	gameObjectScripts[entityID][scriptId]->callInit();
}

void Interface::initializeScript(Script^ script) {
	script->callInit();
}

void Interface::setScriptFieldData(EntityID entityID, ScriptID scriptID, FieldData const& fieldData)
{
	if (!gameObjectScripts->ContainsKey(entityID) || !gameObjectScripts[entityID]->ContainsKey(scriptID)) {
		Logger::error("Failed to set field data for entityID {}", entityID);
		return;
	}

	Script^ script = gameObjectScripts[entityID][scriptID];
	setFieldData(script, fieldData);
}

void Interface::setFieldData(Script^ script, FieldData const& fieldData) {;
	using BindingFlags = System::Reflection::BindingFlags;

	System::Type^ type = script->GetType();
	do {
		array<System::Reflection::FieldInfo^>^ fieldInfos = type->GetFields(BindingFlags::DeclaredOnly | BindingFlags::Instance | BindingFlags::Public | BindingFlags::NonPublic);
		for (int i = 0; i < fieldInfos->Length; ++i) {
			// Ignore private and protected members if it doesn't have the serializablefield attribute
			if (!fieldInfos[i]->IsPublic && fieldInfos[i]->GetCustomAttributes(SerializableField::typeid, true)->Length == 0)
				continue;
			// Field names are always unique
			if (msclr::interop::marshal_as<std::string>(fieldInfos[i]->Name) != fieldData.name)
				continue;
			System::Type^ fieldType = fieldInfos[i]->GetModifiedFieldType()->UnderlyingSystemType;
			// GameObject
			if (fieldType == GameObject::typeid) {
				GameObject^ gameObject = safe_cast<GameObject^>(fieldInfos[i]->GetValue(script));
				if (!gameObject) {
					gameObject = gcnew GameObject(std::get<entt::entity>(fieldData.data));
				}
				fieldInfos[i]->SetValue(script, gameObject);
				return;
			}
			// Struct
			if (IManagedStruct^ managedStruct = dynamic_cast<IManagedStruct^>(fieldInfos[i]->GetValue(script))) {
				// Set the value of the copy
				managedStruct->SetValueFromFieldData(fieldData);
				fieldInfos[i]->SetValue(script, managedStruct);
				return;
			}
			// Component
			if (fieldType->IsSubclassOf(IManagedComponent::typeid)) {
				IManagedComponent^ referencedComponent = safe_cast<IManagedComponent^>(fieldInfos[i]->GetValue(script));
				if (!referencedComponent)
					referencedComponent = safe_cast<IManagedComponent^>(System::Activator::CreateInstance(fieldType));
				if (referencedComponent->LoadDetailsFromEntity(static_cast<unsigned int>(std::get<entt::entity>(fieldData.data))))
					fieldInfos[i]->SetValue(script, referencedComponent);
				return;
			}
			// Script
			if (fieldType->IsSubclassOf(Script::typeid)) {
				// The entity the script is going to reference
				Script^ referencedScript;
				unsigned int referencedEntityID = static_cast<unsigned int>(std::get<entt::entity>(fieldData.data));
				// Find the script from the referenced entity
				if (static_cast<entt::entity>(referencedEntityID) != entt::null && gameObjectScripts->ContainsKey(referencedEntityID)) {
					for each (System::UInt64 scriptId in gameObjectScripts[referencedEntityID]->Keys) {
						if (gameObjectScripts[referencedEntityID][scriptId]->GetType() == fieldType) {
							referencedScript = gameObjectScripts[referencedEntityID][scriptId];
							break;
						}
					}
				}
				fieldInfos[i]->SetValue(script, referencedScript);
				return;
			}
			// Typed Resource ID
			if (fieldType->IsSubclassOf(IManagedResourceID::typeid)) {
				if (!SetTypedResourceIDFromScript<ALL_MANAGED_TYPED_RESOURCE_ID>(fieldData, script, fieldInfos[i])) {
					Logger::warn("Typed resource type in script currently not supported for script setting");
				}
			}
			// Strings
			if (fieldType->Equals(System::String::typeid)) {
				System::String^ string{ safe_cast<System::String^>(fieldInfos[i]->GetValue(script)) };
				string = msclr::interop::marshal_as<System::String^>(std::get<std::string>(fieldData.data));
				fieldInfos[i]->SetValue(script, string);
				return;
			}
			// Primitives
			if (SetScriptPrimitiveFromNativeData<ALL_FIELD_PRIMITIVES>(fieldData, script, fieldInfos[i]))
				return;
			if (fieldType->IsPrimitive)
				Logger::warn("Unknown primitive type used in setting fields currently not supported for script serialization");
			break;
		}
		type = type->BaseType;
	} while (type != nullptr);
	
}

void Interface::addTimeoutDelegate(TimeoutDelegate^ timeoutDelegate) {
	timeoutDelegates->Add(timeoutDelegate);
}

std::unordered_set<ResourceID> Interface::GetHierarchyModifiedScripts(ScriptID scriptId)
{
	std::unordered_set<ResourceID> results{};
	System::Type^ baseType;
	if (availableScripts->ContainsKey(scriptId)) {
		baseType = availableScripts[scriptId]->GetType();
		results.insert(scriptId);
	}
	else if (abstractScriptTypes->ContainsKey(scriptId))
		baseType = abstractScriptTypes[scriptId];
	for each (ScriptID id in availableScripts->Keys) {
		System::Type^ type{ availableScripts[id]->GetType() };
		do {
			if (type == baseType) {
				results.insert(id);
				break;
			}
			type = type->BaseType;
		} while (type != nullptr);
		
	}
	return results;
}

void Interface::update() {
	try {
		for each (System::UInt32 entityID in gameObjectScripts->Keys) {
			for each (System::UInt64 scriptID in gameObjectScripts[entityID]->Keys) {
				EntityData const& entityData{ engine->ecs.registry.get<EntityData>(static_cast<entt::entity>(entityID)) };
				if (!entityData.isActive || entityData.inactiveComponents.count(typeid(Scripts).hash_code())) {
					continue;
				};

				Script^ script = gameObjectScripts[entityID][scriptID];
				script->callUpdate();
			}
		}

		// Handle timeout delegates..
		// Check if timeout expires..
		for each (TimeoutDelegate ^ delegate in timeoutDelegates) {
			if (delegate->timeElapsed >= delegate->duration) {
				executeTimeoutDelegates->Add(delegate);
			}

			delegate->timeElapsed += Time::V_FixedDeltaTime();
		}

		// Execute delegate, then remove from the list..
		for each (TimeoutDelegate ^ delegate in executeTimeoutDelegates) {
			delegate->callback();
			timeoutDelegates->Remove(delegate);
		}

		executeTimeoutDelegates->Clear();

		// Check the create game object queue to handle any game object request at the end of the frame..
		for each (System::Collections::Generic::KeyValuePair<EntityID, ScriptDictionary^> ^ kvp1 in createdGameObjectScripts) {
			for each (System::Collections::Generic::KeyValuePair<ScriptID, Script^> ^ kvp2 in kvp1->Value) {
				EntityID entityID = kvp1->Key;
				ScriptID scriptID = kvp2->Key;

				if (!gameObjectScripts->ContainsKey(entityID))
					gameObjectScripts[entityID] = gcnew ScriptDictionary();

				gameObjectScripts[entityID][scriptID] = kvp2->Value;
			}
		}

		createdGameObjectScripts->Clear();

		// Check the delete game object queue to handle any deletion request at the end of the frame..
		while (deleteGameObjectQueue.Count != 0) {
			EntityID entityToRemove = deleteGameObjectQueue.Dequeue();

			// shouldn't really happen but just in case..
			if (!gameObjectScripts->ContainsKey(entityToRemove)) {
				continue;
			}

			for each (Script ^ script in gameObjectScripts[entityToRemove]->Values) {
				// invokes the exit function before removing it..
				script->callExit();

				// unsubscribe from input manager..
				for each (std::size_t observerId in script->scriptObserverIds) {
					engine->inputManager.unsubscribe<ScriptingInputEvents>(ObserverID{ observerId });
				}

				for each (std::size_t observerId in script->mouseMoveObserverIds) {
					engine->inputManager.unsubscribe<MousePosition>(ObserverID{ observerId });
				}

				for each (std::size_t observerId in script->mouseScrollObserverIds) {
					engine->inputManager.unsubscribe<Scroll>(ObserverID{ observerId });
				}
			}

			// removes it..
			gameObjectScripts[entityToRemove]->Clear();

			// remove from ECS registry..
			engine->ecs.deleteEntity(static_cast<entt::entity>(entityToRemove));
		}
	}
	catch (System::Exception^ exception) {
		Logger::error("{}", Convert(exception->ToString()));		
		Interface::engine->stopSimulation();
	}
}


void Interface::removeEntity(EntityID entityID)
{
	gameObjectScripts->Remove(entityID);
}

void Interface::removeEntityScript(EntityID entityID, ScriptID scriptId)
{
	if (!availableScripts->ContainsKey(scriptId)) {
		Logger::error("Failed to remove script {} for entity {}!", scriptId, entityID);
		return;
	}
	gameObjectScripts[entityID]->Remove(scriptId);
}

void Interface::loadAssembly()
{
	if (assemblyLoadContext) {
		Logger::error("Attempting to load the assembly again?!");
		assert(false);
		return;
	}

	// ========================================================
	// 1. Load C# .dll to assembly context.
	// ========================================================
	assemblyLoadContext = gcnew System::Runtime::Loader::AssemblyLoadContext(nullptr, true);
	
	std::filesystem::path originalPath{ std::filesystem::current_path() };
	const char* path{ runtimePath };
	std::filesystem::current_path(path); // Set to the output directory to load nova-scripts.dll from
	System::IO::FileStream^ scriptLibFile = System::IO::File::Open(
		"Nova-Scripts.dll",
		System::IO::FileMode::Open, System::IO::FileAccess::Read
	);
	std::filesystem::current_path(originalPath); // Reset the path	
	
	assemblyLoadContext->LoadFromStream(scriptLibFile);
	scriptLibFile->Close();

	// ========================================================
	// 2. We first find our loaded nova script assembly.
	// ========================================================
	System::Reflection::Assembly^ novaScriptAssembly = nullptr;

	for each (System::Reflection::Assembly ^ assembly in assemblyLoadContext->Assemblies) {
		if (assembly->GetName()->Name == "Nova-Scripts") {
			novaScriptAssembly = assembly;
			break;
		}
	}

	if (!novaScriptAssembly) {
		Logger::error("Failed to find loaded nova script assembly!");
		return;
	}

	// ========================================================
	// 3. Load all script types in loaded assembly.
	// ========================================================
	
	// We maintain a temporary dictionary mapping class names to script types.
	System::Collections::Generic::Dictionary<System::String^, System::Type^> classNameToScriptType;
	for each (System::Type ^ type in novaScriptAssembly->GetTypes()) {
		if (!type->IsSubclassOf(Script::typeid))
			continue;
		classNameToScriptType.Add(type->Name, type);
	}
	
	// ========================================================
	// 4. We define mapping from Resource IDs to script types.
	// ========================================================
	auto&& scripts = engine->resourceManager.getAllResources<ScriptAsset>();
	for (auto&& scriptId : scripts) {
		auto&& [script, _] = engine->resourceManager.getResource<ScriptAsset>(scriptId);
		assert(script && "Script should always be instantly available.");

		System::String^ className = gcnew System::String(script->getClassName().c_str());
		if (!classNameToScriptType.ContainsKey(className)) {
			Logger::warn("Script asset {} contains invalid class name. Filename and class name probably don't match."
						 "\nThis script is not loaded.", static_cast<std::size_t>(scriptId));
			continue;
		}

		System::Type^ scriptType = classNameToScriptType[className];

		if (!scriptType->IsAbstract) {
			availableScripts->Add(static_cast<std::size_t>(scriptId), safe_cast<Script^>(System::Activator::CreateInstance(scriptType)));
		}
		else
		{
			abstractScriptTypes->Add(static_cast<std::size_t>(scriptId), scriptType);
		}
	}
}

void Interface::unloadAssembly()
{
	if (!assemblyLoadContext)
		return;

	// Clear existing scripts
	if (gameObjectScripts)	
		gameObjectScripts->Clear();
	if (availableScripts)
		availableScripts->Clear();
	if (abstractScriptTypes)
		abstractScriptTypes->Clear();
	if (timeoutDelegates)
		timeoutDelegates->Clear();
	if (executeTimeoutDelegates)
		executeTimeoutDelegates->Clear();
	deleteGameObjectQueue.Clear();
	
	// Clear all input mapping..
	Input::ClearAllKeyMapping();

	// Unload the assembly
	assemblyLoadContext->Unload();
	assemblyLoadContext = nullptr;

	// Garbage Collect existing memory
	System::GC::Collect();
	// Wait from assembly to finish unloading
	System::GC::WaitForPendingFinalizers();
}
