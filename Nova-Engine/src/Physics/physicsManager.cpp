#include "Engine/engine.h"
#include "physicsManager.h"
#include "component.h"
#include "ECS/ECS.h"
#include "Engine/window.h"
#include "ECS/Events.h"
#include "ResourceManager/resourceManager.h"

#include "Profiling.h"
#include <memory>
#include <iostream>
#include <cstdarg>
#include <algorithm>

#include <Jolt/Jolt.h>

// Jolt includes
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyLockInterface.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

namespace {
	// Callback for traces, connect this to your own trace function if you have one
	void TraceImpl(const char* inFMT, ...) {
		// Format the message
		std::va_list list;
		va_start(list, inFMT);
		char buffer[1024];
		vsnprintf(buffer, sizeof(buffer), inFMT, list);
		va_end(list);

		// Print to the TTY
		std::cout << buffer << '\n';
	}

	// Callback for asserts, connect this to your own assert handler if you have one
	bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, unsigned int inLine) {
		// Print to the TTY
		std::cout << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr ? inMessage : "") << '\n';

		// Breakpoint
		return true;
	};

	JPH::RVec3Arg toJPHVec3(glm::vec3 const& glmVec3) {
		return { glmVec3.x, glmVec3.y, glmVec3.z };
	}

	constexpr glm::vec3 toGlmVec3(JPH::RVec3Arg const& jphVec3) {
		return { jphVec3.GetX(), jphVec3.GetY(), jphVec3.GetZ() };
	}

	JPH::Quat toJPHQuat(glm::quat const& glmQuat) {
		return { glmQuat.x, glmQuat.y, glmQuat.z, glmQuat.w };
	}

	constexpr glm::quat toGlmQuat(JPH::Quat const& jphQuat) {
		// because glm built different.
		return { jphQuat.GetW(), jphQuat.GetX(), jphQuat.GetY(), jphQuat.GetZ() };
	}
}

// We need a temp allocator for temporary allocations during the physics update. We're
// pre-allocating 10 MB to avoid having to do allocations during the physics update.
// B.t.w. 10 MB is way too much for this example but it is a typical value you can use.
// If you don't want to pre-allocate you can also use TempAllocatorMalloc to fall back to
// malloc / free.
constexpr std::size_t TEMPORARY_MEMORY = 10 * 1024 * 1024;

constexpr unsigned int maxPhysicsJobs = 2048;
constexpr unsigned int maxPhysicsBarriers = 8;

// This is the max amount of rigid bodies that you can add to the physics system. If you try to add more you'll get an error.
// Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
constexpr unsigned int maxBodies = 1024;

// This determines how many mutexes to allocate to protect rigid bodies from concurrent access. Set it to 0 for the default settings.
constexpr unsigned int numBodyMutexes = 0;

// This is the max amount of body pairs that can be queued at any time (the broad phase will detect overlapping
// body pairs based on their bounding boxes and will insert them into a queue for the narrowphase). If you make this buffer
// too small the queue will fill up and the broad phase jobs will start to do narrow phase work. This is slightly less efficient.
// Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
constexpr unsigned int maxBodyPairs = 1024;

// This is the maximum size of the contact constraint buffer. If more contacts (collisions between bodies) are detected than this
// number then these contacts will be ignored and bodies will start interpenetrating / fall through the world.
// Note: This value is low because this is a simple test. For a real project use something in the order of 10240.
constexpr unsigned int maxContactConstraints = 1024;

PhysicsManager::PhysicsManager(Engine& engine) :
	// we use a placeholder to invoke a function before constructing the rest of the data member.
	placeholder		{ [&](){ 
		// Register allocation hook. In this example we'll just let Jolt use malloc / free but you can override these if you want (see Memory.h).
		// This needs to be done before any other Jolt function is called.
		JPH::RegisterDefaultAllocator();
		return 0;
	}() },

	temp_allocator	{ TEMPORARY_MEMORY },
	job_system		{ maxPhysicsJobs, maxPhysicsBarriers, static_cast<int>(std::thread::hardware_concurrency() / 2U) },

	// The main way to interact with the bodies in the physics system is through the body interface. There is a locking and a non-locking
	// variant of this. We're going to use the locking version (even though we're not planning to access bodies from multiple threads)
	// @ IF SOMETHING SEGFAULTS HERE ITS PROBABLY CAUSE I CALL .GETBODYINTERFACE BEFORE .INIT.
	bodyInterface	{ physicsSystem.GetBodyInterface() },
	debugRenderer	{ engine.renderer },
	registry		{ engine.ecs.registry },
	engine			{ engine },
	contactListener	{ *this }
{
	// Install trace and assert callbacks
	JPH::Trace = TraceImpl;
	JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertFailedImpl;)

	// Create a factory, this class is responsible for creating instances of classes based on their name or hash and is mainly used for deserialization of saved data.
	// It is not directly used in this example but still required.
	JPH::Factory::sInstance = new JPH::Factory();

	// Register all physics types with the factory and install their collision handlers with the CollisionDispatch class.
	// If you have your own custom shape types you probably need to register their handlers with the CollisionDispatch before calling this function.
	// If you implement your own default material (PhysicsMaterial::sDefault) make sure to initialize it before this function or else this function will create one for you.
	JPH::RegisterTypes();

	// Now we can create the actual physics system.
	physicsSystem.Init(maxBodies, numBodyMutexes, maxBodyPairs, maxContactConstraints, broad_phase_layer_interface, object_vs_broadphase_layer_filter, object_vs_object_layer_filter);
	
	//set transformUpdateListener to listen to transfrom events from transform system
	engine.ecs.systemEventDispatcher.sink<TransformUpdateEvent>().connect<&PhysicsManager::transformUpdateListener>(*this);
	
	//Hear construct and destroy functions ------------
	registry.on_construct<Rigidbody>().connect<&PhysicsManager::addBodiesToSystem>(*this);
	registry.on_construct<BoxCollider>().connect<&PhysicsManager::addBodiesToSystem>(*this);
	registry.on_construct<SphereCollider>().connect<&PhysicsManager::addBodiesToSystem>(*this);
	registry.on_construct<CapsuleCollider>().connect<&PhysicsManager::addBodiesToSystem>(*this);
	registry.on_construct<MeshCollider>().connect<&PhysicsManager::addBodiesToSystem>(*this);


	registry.on_destroy<Rigidbody>().connect<&PhysicsManager::removeBodiesFromSystem>(*this);
	registry.on_destroy<BoxCollider>().connect<&PhysicsManager::removeBodiesFromSystem>(*this);
	registry.on_destroy<SphereCollider>().connect<&PhysicsManager::removeBodiesFromSystem>(*this);
	registry.on_destroy<CapsuleCollider>().connect<&PhysicsManager::removeBodiesFromSystem>(*this);
	registry.on_destroy<MeshCollider>().connect<&PhysicsManager::removeBodiesFromSystem>(*this);
	//--------------------------------------------------------------------//


	// We create some primitive shapes to be shared.
	createPrimitiveShapes();

#if 0
	// A body activation listener gets notified when bodies activate and go to sleep
	// Note that this is called from a job so whatever you do here needs to be thread safe.
	// Registering one is entirely optional.
	MyBodyActivationListener body_activation_listener;
	physics_system.SetBodyActivationListener(&body_activation_listener);


#endif
	// A contact listener gets notified when bodies (are about to) collide, and when they separate again.
	// Note that this is called from a job so whatever you do here needs to be thread safe.
	// Registering one is entirely optional.
	physicsSystem.SetContactListener(&contactListener);

	setGravity(gravityStrength);
}

PhysicsManager::~PhysicsManager() {
	JPH::UnregisterTypes();
	delete JPH::Factory::sInstance;
}

void PhysicsManager::simulationInitialise() {
	for (auto&& [entityId, entityData, transform, rigidbody] : registry.view<EntityData, Transform, Rigidbody>().each()) {
		if (rigidbody.bodyId == JPH::BodyID{}) {
			continue;
		}

		bodyInterface.SetLinearVelocity(rigidbody.bodyId, toJPHVec3(rigidbody.initialVelocity));
		rigidbody.velocity = rigidbody.initialVelocity;
		bodyInterface.SetIsSensor(rigidbody.bodyId, rigidbody.isTrigger);
		bodyInterface.ActivateBody(rigidbody.bodyId);
		bodyInterface.SetGravityFactor(rigidbody.bodyId, rigidbody.gravityMultiplier);
	}

	physicsSystem.OptimizeBroadPhase();
}

void PhysicsManager::systemInitialise() {
	for (auto&& [entityId, transform, entityData, rigidbody] : registry.view<Transform, EntityData, Rigidbody>().each()) {
		if (!entityData.isActive) {
			continue;
		}

		// due to the listener system it is possible the the objects have already been created on scene create 
		// if that is the case do not double add objects
		if (rigidbody.bodyId == JPH::BodyID{} && hasRequiredPhysicsComponents(entityId))
			createBody(entityId);
	}

	physicsSystem.OptimizeBroadPhase();
}

void PhysicsManager::clear() {
	bodyInterface.RemoveBodies(createdBodies.data(), static_cast<int>(createdBodies.size()));
	bodyInterface.DestroyBodies(createdBodies.data(), static_cast<int>(createdBodies.size()));
	createdBodies.clear();
	nonRotatableBodies.clear();

	for (auto&& [entityId, rigidbody] : registry.view<Rigidbody>().each()) {
		rigidbody.bodyId = JPH::BodyID{}; // default constructed bodyID is invalid.
	}
}

void PhysicsManager::updatePhysics(float dt) {
#if !defined(NOVA_INSTALLER)
	ZoneScoped;
#endif

	// =============================================================
	// 1. Update all physics body to the current object's transform.
	// @TODO: Don't update every frame! Only update when there is a change in transform. 
	// =============================================================
	for (auto&& [entityId, transform, entityData, rigidbody] : registry.view<Transform, EntityData, Rigidbody>().each()) {
		if (rigidbody.bodyId == JPH::BodyID{ JPH::BodyID::cInvalidBodyID }) {
			continue;
		}

		// our engine doesnt move static objects.
		if (rigidbody.motionType == JPH::EMotionType::Static) {
			continue;
		}

		bodyInterface.SetPositionAndRotation(rigidbody.bodyId, toJPHVec3(transform.position + rigidbody.offset), toJPHQuat(transform.rotation), JPH::EActivation::Activate);

		// @TODO: Optimise?
		if (rigidbody.dynamicCollider) {
			JPH::ScaledShape* shape = recreateScaledShape(entityId, transform, rigidbody);
			bodyInterface.SetShape(rigidbody.bodyId, shape, false, JPH::EActivation::Activate);
		}
	}

	// run physics simulation.
	physicsSystem.Update(dt, 1, &temp_allocator, &job_system);	

	// =============================================================
	// 2. Update all object transform after running physics simulation.
	// @TODO: Don't update every frame! Only update when there is a change in transform.
	// =============================================================

	for (auto&& [entityId, transform, entityData, rigidbody] : registry.view<Transform, EntityData, Rigidbody>().each()) {
		if (rigidbody.bodyId == JPH::BodyID{ JPH::BodyID::cInvalidBodyID }) {
			continue;
		}

		if (rigidbody.motionType == JPH::EMotionType::Static) {
			continue;
		}

		JPH::Vec3 pos;
		JPH::Quat rotation;

		if (rigidbody.isRotatable) {
			bodyInterface.GetPositionAndRotation(rigidbody.bodyId, pos, rotation);
			transform.position = toGlmVec3(pos) - rigidbody.offset;
			transform.rotation = toGlmQuat(rotation);
		}
		else {
			pos = bodyInterface.GetPosition(rigidbody.bodyId);
			transform.position = toGlmVec3(pos) - rigidbody.offset;
		}

		rigidbody.velocity = toGlmVec3(bodyInterface.GetLinearVelocity(rigidbody.bodyId));
	}

#if 0
	for (auto bodyId : nonRotatableBodies) {
		bodyInterface.SetAngularVelocity(bodyId, JPH::Vec3::sZero());
	}
#endif

	// =============================================================
	// 3. Handle collision request..
	// =============================================================

	std::lock_guard lock{ onCollisionMutex };

	while (onCollisionEnter.size()) {
		auto [entityOne, entityTwo] = onCollisionEnter.front();
		onCollisionEnter.pop();
		
		engine.scriptingAPIManager.onCollisionEnter(entityOne, entityTwo);
	}

	while (onCollisionExit.size()) {
		auto [entityOne, entityTwo] = onCollisionExit.front();
		onCollisionExit.pop();

		engine.scriptingAPIManager.onCollisionExit(entityOne, entityTwo);
	}
}

void PhysicsManager::debugRender() {

	//i think the best way to handle debugRender is that it is independent of whether the simulation is running. If the simulation is running
	// it is assumed that physics update handles transformation changes else look at transform and draw it.
	constexpr JPH::BodyManager::DrawSettings debugDrawSettings {};
	physicsSystem.DrawBodies(debugDrawSettings, &debugRenderer);
}

void PhysicsManager::resetPhysicsState()
{
	//clear the stack, do not allow anymore updates,
	//reset transformation data.
	transformUpdateStack = std::stack<entt::entity>();

	clear();
	systemInitialise();
}

void PhysicsManager::transformUpdateListener(TransformUpdateEvent const& event)
{
	if (!hasRequiredPhysicsComponents(event.entityID))
	{
		return;
	}

	transformUpdateStack.push(event.entityID);
}

void PhysicsManager::updateTransformBodies()
{
	// Look at transformUpdateListener, is filled from transform system.
	while (!transformUpdateStack.empty())
	{
		entt::entity entity = transformUpdateStack.top();
		transformUpdateStack.pop();

		Transform const& transform = registry.get<Transform>(entity);
		Rigidbody const& rigidbody = registry.get<Rigidbody>(entity);

		if (rigidbody.bodyId == JPH::BodyID{ JPH::BodyID::cInvalidBodyID }) {
			continue;
		}

		bodyInterface.SetPositionAndRotation(rigidbody.bodyId, toJPHVec3(transform.position + rigidbody.offset), toJPHQuat(transform.rotation), JPH::EActivation::Activate);
	}

	// can optimise later to listen to patch events
	for (auto&& [entityId, transform, rigidbody] : registry.view<Transform, Rigidbody>().each())
	{
		// If it's a mesh collider, I don't want to recalculate shape.. expensive..
		if (registry.any_of<MeshCollider>(entityId)) {
			continue;
		}

		JPH::EActivation type;
		if (bodyInterface.IsActive(rigidbody.bodyId))
		{
			type = JPH::EActivation::Activate;
		}
		else
		{
			type = JPH::EActivation::DontActivate;
		}

		JPH::ScaledShape* shape = recreateScaledShape(entityId, transform, rigidbody);
		bodyInterface.SetShape(rigidbody.bodyId, shape, false, type);
		bodyInterface.SetIsSensor(rigidbody.bodyId, rigidbody.isTrigger);
	}
}

void PhysicsManager::addBodiesToSystem(entt::registry&, entt::entity entityID)
{
	if (!hasRequiredPhysicsComponents(entityID))
	{
		return;
	}

	createBody(entityID);
}

void PhysicsManager::removeBodiesFromSystem(entt::registry&, entt::entity entityID) {
	auto&& [transform, rigidBody]= registry.try_get<Transform, Rigidbody>(entityID);

	if (rigidBody == nullptr)
	{
		return;
	}

	if (rigidBody->bodyId == JPH::BodyID{})
	{
		return;
	}

	auto it = std::find(createdBodies.begin(), createdBodies.end(), rigidBody->bodyId);
	std::swap(*it, *(createdBodies.end() - 1));
	createdBodies.erase(createdBodies.end() - 1);

	bodyInterface.RemoveBody(rigidBody->bodyId);
	bodyInterface.DestroyBody(rigidBody->bodyId);

	rigidBody->bodyId = JPH::BodyID{};
}

bool PhysicsManager::isBodyAlive(JPH::BodyID id)
{
	
	auto it = std::find(createdBodies.begin(), createdBodies.end(), id);

	if (it == createdBodies.end())
	{
		return false;
	}

	return true;
}

void PhysicsManager::createPrimitiveShapes() {
	// ===========================================
	// 1. Constructing a box shape.
	// ===========================================
	JPH::BoxShapeSettings boxSettings { JPH::Vec3{ 1.f, 1.f, 1.f } };
	boxSettings.SetEmbedded();	// box settings is allocated on the stack, and this class is actually a smart pointer that does reference counting 
								// (for some reason) so we have to disable it

	box = boxSettings.Create().Get(); // hehe i ignore error.

	// ===========================================
	// 2. Constructing a sphere shape.
	// ===========================================
	JPH::SphereShapeSettings sphereSettings { 0.5f };
	sphereSettings.SetEmbedded(); // whatever i just yapped at the top

	sphere = sphereSettings.Create().Get();


	// ===========================================
	// 3. Constructing a capsule shape.
	// ===========================================
	JPH::CapsuleShapeSettings capsuleSettings{ 0.5f, 0.5f };
	capsuleSettings.SetEmbedded();

	capsule = capsuleSettings.Create().Get();
}

void PhysicsManager::createBody(entt::entity const& entityID)
{
	Transform const& transform = registry.get<Transform>(entityID);
	Rigidbody& rigidBody = registry.get<Rigidbody>(entityID);

	JPH::ScaledShape* shape = recreateScaledShape(entityID, transform, rigidBody);

	if (!shape) {
		Logger::error("Failed to create shape for entity: {}", static_cast<unsigned int>(entityID));
		return;
	}

	JPH::BodyCreationSettings bodySettings{
		shape,	// scaled shape
		toJPHVec3(transform.position),															// position
		toJPHQuat(transform.rotation),															// rotation (in quartenions)
		rigidBody.motionType,																	// motion type
		static_cast<JPH::ObjectLayer>(rigidBody.layer)											// in which layer?
	};

	// We now specify mass.. (if it's not static..)
	if (rigidBody.motionType != JPH::EMotionType::Static) {
		JPH::MassProperties massProperties;
		massProperties.ScaleToMass(rigidBody.mass == 0 ? 1.f : rigidBody.mass); //actual mass in kg

		bodySettings.mMassPropertiesOverride = massProperties;
		bodySettings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
	}
	
	if (rigidBody.motionType == JPH::EMotionType::Kinematic && rigidBody.isTrigger) {
		bodySettings.mCollideKinematicVsNonDynamic = true;
	}

	JPH::EActivation activationType = engine.isInSimulationMode() ? JPH::EActivation::Activate : JPH::EActivation::DontActivate;

	JPH::BodyID bodyId = bodyInterface.CreateAndAddBody(
		bodySettings,
		activationType //should always be inactive unless its simulation step
	);

	bodyInterface.SetUserData(bodyId, static_cast<unsigned>(entityID));
	bodyInterface.SetLinearVelocity(bodyId, toJPHVec3(rigidBody.initialVelocity));
	bodyInterface.SetIsSensor(bodyId, rigidBody.isTrigger);
	bodyInterface.SetRestitution(bodyId, rigidBody.restitution);
	bodyInterface.SetGravityFactor(bodyId, rigidBody.gravityMultiplier);

	if (rigidBody.motionQuality == Rigidbody::MotionQuality::Continuous) {
		bodyInterface.SetMotionQuality(bodyId, JPH::EMotionQuality::LinearCast);
	}

	createdBodies.push_back(bodyId);

	rigidBody.bodyId = bodyId;

	// Record not rotatable objects.. (non static)
	if (rigidBody.motionType != JPH::EMotionType::Static) {
		nonRotatableBodies.push_back(bodyId);
	}
}

bool PhysicsManager::hasRequiredPhysicsComponents(entt::entity const& entityID) {
	return registry.any_of<BoxCollider, SphereCollider, MeshCollider, CapsuleCollider>(entityID) && registry.all_of<Rigidbody>(entityID);
}

JPH::ScaledShape* PhysicsManager::recreateScaledShape(entt::entity entity, Transform const& transform, Rigidbody& rigidBody) {
	auto&& [
		meshRenderer, 
		boxCollider,
		sphereCollider, 
		capsuleCollider, 
		meshCollider
	] = registry.try_get<MeshRenderer, BoxCollider, SphereCollider, CapsuleCollider, MeshCollider>(entity);

	// Create and add body based on entity's component.
	JPH::ScaledShape* shape = nullptr;

	// Calculate appropriate scale.
	glm::vec3 scale;
	glm::vec3 transformScale = rigidBody.toScaleWithTransform ? transform.scale : glm::vec3{ 1.f, 1.f, 1.f };

	// if have model use model scale, else init without model
	if (meshRenderer != nullptr)
	{
		auto [model, _] = engine.resourceManager.getResource<Model>(meshRenderer->modelId);

		if (!model || model->maxDimension == 0) {
			scale = transformScale;
		}
		else {
			scale = transformScale * model->scale;
		}
	}
	else
	{
		scale = transformScale;
	}

	// Alright here we go have to list down all the possible collider type cause we dk which one is it. :P, 
	// we only support one type of collider per entt now i think
	if (boxCollider != nullptr)
	{
		shape = new JPH::ScaledShape(box, box->MakeScaleValid(toJPHVec3(boxCollider->shapeScale * scale)));
		rigidBody.offset = boxCollider->offset;
	}
	else if (sphereCollider != nullptr)
	{
		shape = new JPH::ScaledShape(sphere, sphere->MakeScaleValid(toJPHVec3(glm::vec3(sphereCollider->radius * scale))));
		rigidBody.offset = sphereCollider->offset;
	}
	else if (capsuleCollider != nullptr) {
		shape = new JPH::ScaledShape(capsule, capsule->MakeScaleValid(toJPHVec3(glm::vec3{ capsuleCollider->shapeScale * scale })));
		rigidBody.offset = capsuleCollider->offset;
	}
	else if (meshRenderer && meshCollider) {
		rigidBody.offset = { 0.f, 0.f, 0.f };

		auto [model, _] = engine.resourceManager.getResource<Model>(meshRenderer->modelId);

		if (model) {
			unsigned int vertexOffset = 0;

			// Get the vertices and indices..
			JPH::VertexList vertexList;
			JPH::IndexedTriangleList indexedTriangleList;

			for (auto&& meshData : model->meshes) {
				for (glm::vec3 const& localPosition : meshData.positions) {
					vertexList.push_back({ localPosition.x, localPosition.y, localPosition.z });
				}

				// map triangle soup with corresponding indices in each mesh
				for (size_t i = 0; i < meshData.indices.size(); i += 3) {
					indexedTriangleList.push_back({
						meshData.indices[i + 0] + vertexOffset,
						meshData.indices[i + 1] + vertexOffset,
						meshData.indices[i + 2] + vertexOffset
					});
				}

				vertexOffset += static_cast<unsigned int>(meshData.positions.size());
			}

			// Create the shape..
			JPH::MeshShapeSettings meshShape{ vertexList, indexedTriangleList };
			meshShape.SetEmbedded();

			auto&& result = meshShape.Create();

			if (result.HasError()) {
				Logger::error("Error creating mesh shape settings.. {}", result.GetError());
				return nullptr;
			}
			else {
				glm::vec3 shapeScale = meshCollider->shapeScale * scale;

				bool anyComponentZero = false;
				for (int i = 0; i < 3; ++i) {
					if (shapeScale[i] == 0.0f) {
						anyComponentZero = true;
						break;
					}
				}

				if (anyComponentZero) {
					shapeScale = glm::vec3{ 1.f, 1.f, 1.f };
				}

				shape = new JPH::ScaledShape(result.Get(), toJPHVec3(shapeScale));
			}
		}
	}

	return shape;
}

void PhysicsManager::submitCollisionEnter(entt::entity entityOne, entt::entity entityTwo) {
	std::lock_guard lock{ onCollisionMutex };
	onCollisionEnter.push({ entityOne, entityTwo });
}

void PhysicsManager::submitCollisionExit(entt::entity entityOne, entt::entity entityTwo) {
	std::lock_guard lock{ onCollisionMutex };
	onCollisionExit.push({ entityOne, entityTwo });
}


void PhysicsManager::SetPhysicsLayer(Rigidbody& rigidbody, Rigidbody::Layer physicsLayer)
{
	if (rigidbody.bodyId == JPH::BodyID{}) {
		return;
	}

	rigidbody.layer = physicsLayer;

	bodyInterface.SetObjectLayer(rigidbody.bodyId, static_cast<JPH::ObjectLayer>(physicsLayer));
}

PhysicsRay PhysicsManager::getRayFromMouse() const {
	glm::vec3 farClipPos = { engine.window.getClipSpacePos(), 1.f };
	glm::vec3 nearClipPos = { farClipPos.x, farClipPos.y, -1.f };

	glm::vec3 farWorldPos = engine.renderer.getGameCamera().clipToWorldSpace(farClipPos);
	glm::vec3 nearWorldPos = engine.renderer.getGameCamera().clipToWorldSpace(nearClipPos);

	glm::vec3 raycastDirection = glm::normalize(farWorldPos - nearWorldPos);

	return { nearWorldPos, raycastDirection };
}

std::optional<PhysicsRayCastResult> PhysicsManager::rayCast(PhysicsRay ray, float maxDistance, std::vector<entt::entity> const& ignoredEntities) {
	auto&& narrowPhaseQuery = physicsSystem.GetNarrowPhaseQuery();
	
	JPH::RayCastResult rayCastResult;
	glm::vec3 distanceVector = glm::normalize(ray.direction) * maxDistance;

	JPH::IgnoreMultipleBodiesFilter ignoreFilter;
	
	for (entt::entity entity : ignoredEntities) {
		Rigidbody* rigidbody = registry.try_get<Rigidbody>(entity);

		if (rigidbody) {
			ignoreFilter.IgnoreBody(rigidbody->bodyId);
		}
	}

	if (narrowPhaseQuery.CastRay(JPH::RRayCast{ toJPHVec3(ray.origin), toJPHVec3(distanceVector) }, rayCastResult, {}, {}, ignoreFilter)) {
		JPH::BodyID bodyId = rayCastResult.mBodyID;

		entt::entity entity = static_cast<entt::entity>(bodyInterface.GetUserData(bodyId));
		glm::vec3 collisionPoint = ray.origin + distanceVector * rayCastResult.mFraction;
	
		JPH::BodyLockRead lock(physicsSystem.GetBodyLockInterfaceNoLock(), rayCastResult.mBodyID); //goofy ahh engine, Not lock here
		glm::vec3 hitSurfaceNormal = toGlmVec3(lock.GetBody().GetWorldSpaceSurfaceNormal(rayCastResult.mSubShapeID2, toJPHVec3(collisionPoint)));

		return PhysicsRayCastResult{ entity, collisionPoint, hitSurfaceNormal};
	}

	return std::nullopt;
}
std::optional<PhysicsRayCastResult> PhysicsManager::rayCast(PhysicsRay ray, float maxDistance, std::vector<uint8_t> const& layerMask) {
	auto&& narrowPhaseQuery = physicsSystem.GetNarrowPhaseQuery();
	JPH::RayCastResult rayCastResult;
	glm::vec3 distanceVector = glm::normalize(ray.direction) * maxDistance;
	RayCastLayerMaskFilterImpl layerMaskFilter(layerMask);

	// Object Layer
	if (narrowPhaseQuery.CastRay(JPH::RRayCast{ toJPHVec3(ray.origin), toJPHVec3(distanceVector) }, rayCastResult, layerMaskFilter)) {
		JPH::BodyID bodyId = rayCastResult.mBodyID;

		entt::entity entity = static_cast<entt::entity>(bodyInterface.GetUserData(bodyId));
		glm::vec3 collisionPoint = ray.origin + distanceVector * rayCastResult.mFraction;
		
		JPH::BodyLockRead lock(physicsSystem.GetBodyLockInterfaceNoLock(), rayCastResult.mBodyID); //goofy ahh engine, Not lock here
		glm::vec3 hitSurfaceNormal = toGlmVec3(lock.GetBody().GetWorldSpaceSurfaceNormal(rayCastResult.mSubShapeID2, toJPHVec3(collisionPoint)));

		return PhysicsRayCastResult{ entity, collisionPoint, hitSurfaceNormal};
	}

	return std::nullopt;
}


void PhysicsManager::addForce(Rigidbody const& rigidbody, glm::vec3 forceVector) {
	// attempts to retrieve the underlying body id..
	if (rigidbody.bodyId == JPH::BodyID{}) {
		return;
	}

	bodyInterface.AddForce(rigidbody.bodyId, toJPHVec3(forceVector));
}

void PhysicsManager::addImpulse(Rigidbody const& rigidbody, glm::vec3 forceVector){
	// attempts to retrieve the underlying body id..
	if (rigidbody.bodyId == JPH::BodyID{}) {
		return;
	}

	bodyInterface.AddImpulse(rigidbody.bodyId, toJPHVec3(forceVector));
}

void PhysicsManager::setVelocity(Rigidbody& rigidbody, glm::vec3 velocity) {
	// attempts to retrieve the underlying body id..
	if (rigidbody.bodyId == JPH::BodyID{}) {
		return;
	}

	bodyInterface.SetLinearVelocity(rigidbody.bodyId, toJPHVec3(velocity));
	rigidbody.velocity = velocity;
}


//Access motion properties after locking physics bodies, have to lock first i think,
// It appears that Jolt dont allow you to modify motion properties without locking first. I assume is a safety feature
void PhysicsManager::setVelocityLimits(Rigidbody& rigidbody, float maxVelocity) {
	// attempts to retrieve the underlying body id..
	if (rigidbody.bodyId == JPH::BodyID{}) {
		return;
	}

	//there is also body no lock interface physicsSystem.GetBodyLockInterfacNoLock()
	JPH::BodyLockWrite lock(physicsSystem.GetBodyLockInterface(), rigidbody.bodyId);

	if (lock.Succeeded())
	{
		JPH::MotionProperties* motionproperites = lock.GetBody().GetMotionProperties();
		motionproperites->SetMaxLinearVelocity(maxVelocity);
	}
	
}

std::optional<float> PhysicsManager::getVelocityLimits(Rigidbody& rigidbody) {
	// attempts to retrieve the underlying body id..
	if (rigidbody.bodyId == JPH::BodyID{}) {
		return std::nullopt;
	}

	JPH::BodyLockWrite lock(physicsSystem.GetBodyLockInterface(), rigidbody.bodyId);

	if (lock.Succeeded())
	{
		JPH::MotionProperties* motionproperites = lock.GetBody().GetMotionProperties();
		return motionproperites->GetMaxLinearVelocity();
	}

	return std::nullopt;
}


//Access motion properties after locking physics bodies, have to lock first i think,
// It appears that Jolt dont allow you to modify motion properties without locking first. I assume is a safety feature
void PhysicsManager::setMaxAngularVelocityLimits(Rigidbody& rigidbody, float maxVelocity) {
	// attempts to retrieve the underlying body id..
	if (rigidbody.bodyId == JPH::BodyID{}) {
		return;
	}

	//there is also body no lock interface physicsSystem.GetBodyLockInterfacNoLock()
	JPH::BodyLockWrite lock(physicsSystem.GetBodyLockInterface(), rigidbody.bodyId);

	if (lock.Succeeded())
	{
		JPH::MotionProperties* motionproperites = lock.GetBody().GetMotionProperties();
		motionproperites->SetMaxAngularVelocity(maxVelocity);
	}

}


std::optional<float> PhysicsManager::getAngularVelocityLimits(Rigidbody& rigidbody) {
	// attempts to retrieve the underlying body id..
	if (rigidbody.bodyId == JPH::BodyID{}) {
		return std::nullopt;
	}

	JPH::BodyLockWrite lock(physicsSystem.GetBodyLockInterface(), rigidbody.bodyId);

	if (lock.Succeeded())
	{
		JPH::MotionProperties* motionproperites = lock.GetBody().GetMotionProperties();
		return motionproperites->GetMaxLinearVelocity();
	}

	return std::nullopt;
}

//Access motion properties after locking physics bodies, have to lock first i think,
// It appears that Jolt dont allow you to modify motion properties without locking first. I assume is a safety feature
void PhysicsManager::setLinearDamping(Rigidbody& rigidbody, float dampingValue) {
	// attempts to retrieve the underlying body id..
	if (rigidbody.bodyId == JPH::BodyID{}) {
		return;
	}

	//there is also body no lock interface physicsSystem.GetBodyLockInterfacNoLock()
	JPH::BodyLockWrite lock(physicsSystem.GetBodyLockInterface(), rigidbody.bodyId);

	if (lock.Succeeded())
	{
		JPH::MotionProperties* motionproperites = lock.GetBody().GetMotionProperties();
		motionproperites->SetLinearDamping(dampingValue);
	}

}

//Access motion properties after locking physics bodies, have to lock first i think,
// It appears that Jolt dont allow you to modify motion properties without locking first. I assume is a safety feature
std::optional<float> PhysicsManager::getLinearDamping(Rigidbody& rigidbody) {
	// attempts to retrieve the underlying body id..
	if (rigidbody.bodyId == JPH::BodyID{}) {
		return std::nullopt;
	}

	//there is also body no lock interface physicsSystem.GetBodyLockInterfacNoLock()
	JPH::BodyLockWrite lock(physicsSystem.GetBodyLockInterface(), rigidbody.bodyId);

	if (lock.Succeeded())
	{
		JPH::MotionProperties* motionproperites = lock.GetBody().GetMotionProperties();
		return motionproperites->GetLinearDamping();
	}

	return std::nullopt;

}

//Access motion properties after locking physics bodies, have to lock first i think,
// It appears that Jolt dont allow you to modify motion properties without locking first. I assume is a safety feature
void PhysicsManager::setAngularDamping(Rigidbody& rigidbody, float dampingValue) {
	// attempts to retrieve the underlying body id..
	if (rigidbody.bodyId == JPH::BodyID{}) {
		return;
	}

	//there is also body no lock interface physicsSystem.GetBodyLockInterfacNoLock()
	JPH::BodyLockWrite lock(physicsSystem.GetBodyLockInterface(), rigidbody.bodyId);

	if (lock.Succeeded())
	{
		JPH::MotionProperties* motionproperites = lock.GetBody().GetMotionProperties();
		motionproperites->SetAngularDamping(dampingValue);
	}

}


void PhysicsManager::setAngularVelocity(Rigidbody& rigidbody, glm::vec3 velocity)
{
	if (rigidbody.bodyId == JPH::BodyID{}) {
		return;
	}

	bodyInterface.SetAngularVelocity(rigidbody.bodyId, toJPHVec3(velocity));
	rigidbody.angularVelocity = velocity;
}


//Access motion properties after locking physics bodies, have to lock first i think,
// It appears that Jolt dont allow you to modify motion properties without locking first. I assume is a safety feature
std::optional<float> PhysicsManager::getAngularDamping(Rigidbody& rigidbody) {
	// attempts to retrieve the underlying body id..
	if (rigidbody.bodyId == JPH::BodyID{}) {
		return std::nullopt;
	}

	//there is also body no lock interface physicsSystem.GetBodyLockInterfacNoLock()
	JPH::BodyLockWrite lock(physicsSystem.GetBodyLockInterface(), rigidbody.bodyId);

	if (lock.Succeeded())
	{
		JPH::MotionProperties* motionproperites = lock.GetBody().GetMotionProperties();
		return motionproperites->GetAngularDamping();
	}

	return std::nullopt;

}

void PhysicsManager::setRotation(Rigidbody& rigidbody, glm::quat quaternion)
{
	if (rigidbody.bodyId == JPH::BodyID{}) {
		return;
	}

	if (rigidbody.motionType == JPH::EMotionType::Static)
	{
		bodyInterface.SetRotation(rigidbody.bodyId, toJPHQuat(quaternion), JPH::EActivation::DontActivate);

	}
	else
	{
		bodyInterface.SetRotation(rigidbody.bodyId, toJPHQuat(quaternion), JPH::EActivation::Activate);

	}
}

float PhysicsManager::getGravity()
{
	return gravityStrength;
}

void PhysicsManager::setGravity(float value) {
	gravityStrength = value;
	physicsSystem.SetGravity(JPH::Vec3{ 0.f, -gravityStrength, 0.f });
}

void PhysicsManager::setGravityFactor(Rigidbody& rigidbody, float value) {
	bodyInterface.SetGravityFactor(rigidbody.bodyId, value);
	rigidbody.gravityMultiplier = value;
}

void PhysicsManager::setMass(Rigidbody& rigidbody, float value) {
	JPH::BodyLockWrite lock(physicsSystem.GetBodyLockInterfaceNoLock(), rigidbody.bodyId);
	
	if (lock.Succeeded())
	{
		JPH::MotionProperties* motionproperites = lock.GetBody().GetMotionProperties();
		motionproperites->ScaleToMass(value);
		
		rigidbody.mass = value;

	}
	
}


float PhysicsManager::getMass(Rigidbody& rigidbody) {
	//JPH::BodyLockWrite lock(physicsSystem.GetBodyLockInterfaceNoLock(), rigidbody.bodyId);

	//if (lock.Succeeded())
	//{
	//	JPH::MotionProperties* motionproperites = lock.GetBody().GetMotionProperties();
	//	return (1/motionproperites->GetInverseMass());

	//}

	return rigidbody.mass;

}