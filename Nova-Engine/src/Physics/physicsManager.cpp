#include "physicsManager.h"

#include <iostream>
#include <cstdarg>

#include <Jolt/Jolt.h>

// Jolt includes
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
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

PhysicsManager::PhysicsManager(Renderer& renderer) :
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
	debugRenderer	{ renderer }
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
	
	// We create some primitive shapes to be shared.
	createPrimitiveShapes();

#if 0
	// A body activation listener gets notified when bodies activate and go to sleep
	// Note that this is called from a job so whatever you do here needs to be thread safe.
	// Registering one is entirely optional.
	MyBodyActivationListener body_activation_listener;
	physics_system.SetBodyActivationListener(&body_activation_listener);

	// A contact listener gets notified when bodies (are about to) collide, and when they separate again.
	// Note that this is called from a job so whatever you do here needs to be thread safe.
	// Registering one is entirely optional.
	MyContactListener contact_listener;
	physics_system.SetContactListener(&contact_listener);
#endif

	// We create 2 body for testing..
	JPH::BodyCreationSettings bodySettings {
		box,						// shape
		{},							// position
		JPH::Quat::sIdentity(),		// rotation (in quartenions)
		JPH::EMotionType::Static,	// motion type
		Layers::NON_MOVING			// in which layer?
	};

	bodyInterface.CreateAndAddBody(bodySettings, JPH::EActivation::Activate);

	bodySettings.mPosition = { 0.f, 2.f, 0.f };

	bodyInterface.CreateAndAddBody(bodySettings, JPH::EActivation::Activate);

}

PhysicsManager::~PhysicsManager() {
	JPH::UnregisterTypes();
	delete JPH::Factory::sInstance;
}

void PhysicsManager::initialise() {
	// Retrieves all 
}

void PhysicsManager::clear() {

}

void PhysicsManager::update(float dt) {
	physicsSystem.Update(dt, 1, &temp_allocator, &job_system);	
}

void PhysicsManager::debugRender() {
	constexpr JPH::BodyManager::DrawSettings debugDrawSettings {};
	physicsSystem.DrawBodies(debugDrawSettings, &debugRenderer);
}

void PhysicsManager::createPrimitiveShapes() {
	// ===========================================
	// 1. Constructing a box shape.
	// ===========================================
	JPH::BoxShapeSettings boxSettings { JPH::Vec3{ 0.5f, 0.5f, 0.5f } };
	boxSettings.SetEmbedded();	// box settings is allocated on the stack, and this class is actually a smart pointer that does reference counting 
								// (for some reason) so we have to disable it

	box = boxSettings.Create().Get(); // hehe i ignore error.

	// ===========================================
	// 2. Constructing a sphere shape.
	// ===========================================
	JPH::SphereShapeSettings sphereSettings { 0.5f };
	sphereSettings.SetEmbedded(); // whatever i just yapped at the top

	sphere = sphereSettings.Create().Get();
}
