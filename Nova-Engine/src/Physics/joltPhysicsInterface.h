#pragma once

#include <Jolt/Jolt.h>

#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>

#include <iostream>

// Layer that objects can be in, determines which other objects it can collide with
// Typically you at least want to have 1 layer for moving bodies and 1 layer for static bodies, but you can have more
// layers if you want. E.g. you could have a layer for high detail collision (which is not used by the physics simulation
// but only if you do collision testing).
namespace Layers {
	static constexpr JPH::ObjectLayer NON_MOVING = 0;
	static constexpr JPH::ObjectLayer MOVING = 1;
	static constexpr JPH::ObjectLayer WALL = 2;
	static constexpr JPH::ObjectLayer ITEM = 3;
	static constexpr JPH::ObjectLayer NUM_LAYERS = 4;
};

// Each broadphase layer results in a separate bounding volume tree in the broad phase. You at least want to have
// a layer for non-moving and moving objects to avoid having to update a tree full of static objects every frame.
// You can have a 1-on-1 mapping between object layers and broadphase layers (like in this case) but if you have
// many object layers you'll be creating many broad phase trees, which is not efficient. If you want to fine tune
// your broadphase layers define JPH_TRACK_BROADPHASE_STATS and look at the stats reported on the TTY.
namespace BroadPhaseLayers
{
	static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
	static constexpr JPH::BroadPhaseLayer MOVING(1);
	static constexpr JPH::BroadPhaseLayer WALL(2);
	static constexpr JPH::BroadPhaseLayer ITEM(3);
	static constexpr JPH::uint NUM_LAYERS(4);
};

// BroadPhaseLayerInterface implementation
// This defines a mapping between object and broadphase layers.
class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
{
public:
	BPLayerInterfaceImpl() {
		// Create a mapping table from object to broad phase layer
		mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
		mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
		mObjectToBroadPhase[Layers::WALL] = BroadPhaseLayers::WALL;
		mObjectToBroadPhase[Layers::ITEM] = BroadPhaseLayers::ITEM;
	}

	JPH::uint GetNumBroadPhaseLayers() const final {
		return BroadPhaseLayers::NUM_LAYERS;
	}

	JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const final {
		JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
		return mObjectToBroadPhase[inLayer];
	}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
	/// Get the user readable name of a broadphase layer (debugging purposes)
	const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const final {
		switch ((JPH::BroadPhaseLayer::Type)inLayer)
		{
		case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:	return "NON_MOVING";
		case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:		return "MOVING";
		case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::WALL:        return "WALL";
		case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::ITEM:        return "ITEM";
		default:														JPH_ASSERT(false); return "INVALID";
		}
	}
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

private:
	JPH::BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];
};

/// Class that determines if an object layer can collide with a broadphase layer
class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
	bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const final
	{
		switch (inLayer1)
		{
		case Layers::WALL:
		case Layers::NON_MOVING:
			return inLayer2 == BroadPhaseLayers::MOVING || inLayer2 == BroadPhaseLayers::ITEM; // Non moving only collides with moving
		case Layers::MOVING:
			return inLayer2 != BroadPhaseLayers::ITEM;	 // Moving collides with everything except item
		case Layers::ITEM:
			return inLayer2 == BroadPhaseLayers::NON_MOVING;
		default:
			JPH_ASSERT(false);
			return false;
		}
	}
};

/// Class that determines if two object layers can collide
class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter
{
public:
	bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const final
	{
		switch (inObject1)
		{
		case Layers::WALL:
		case Layers::NON_MOVING:
			return inObject2 == Layers::MOVING || inObject2 == Layers::ITEM; // Non moving only collides with moving
		case Layers::MOVING:
			return inObject2 != Layers::ITEM;   // Moving collides with everything except item
		case Layers::ITEM:
			return inObject2 == Layers::NON_MOVING;
		default:
			JPH_ASSERT(false);
			return false;
		}
	}
};

/// Class that determines if raycast layer 
class RayCastLayerMaskFilterImpl : public JPH::BroadPhaseLayerFilter
{
public:
	RayCastLayerMaskFilterImpl(std::vector<uint8_t> const& layerMask) : layerMask{ layerMask } {}
public:
	bool ShouldCollide(JPH::BroadPhaseLayer inLayer) const final {
		if (layerMask.empty())
			return true;
		uint8_t layer = static_cast<uint8_t>(inLayer);
		return std::find(std::begin(layerMask), std::end(layerMask), layer) != std::end(layerMask);
	}
private:
	std::vector<uint8_t> layerMask;
};