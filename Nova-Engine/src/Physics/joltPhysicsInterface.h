#pragma once

#include <Jolt/Jolt.h>

#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>

#include <iostream>


//NOTE: This is our way of creating rules for collision detection here is a quick breakdown of different types of layer
// NON_MOVING -> Default Setting, should collide with everything - not many things in the game should be in this category cause of special interaction rules
// Moving -> Generic moving objects, Player/NPC should be here, should collide with (enviromental colliders) such as wall and floors.
// Wall -> (Enviromental colliders) Collides with everything that is moving and used for special interaction rules with throwables, player and items.
// Items -> Interactable Items like Ichor, should interact with most enviromental colliders and not interact with any npc/player colliders. Cause use for interactive objects as well
// ENEMY_HURTSPOT -> Special Colliders used for detect where player's weapons hit the enemy, should not collide with itself to save collision detection cycles
// Item Interactor -> only need to consider items for collision detection purposes.
// Floor -> (Enviromental colliders) used for player to detect floor.
// PROPS -> (Enviromental colliders) objects around the room that is collidable should be in this category
// PlayerGhost -> Allow player to collide with environment colliders but not moving category
// AttackColliders -> Collides with player and enemy colliders, and environments , use for thrown weapons and attack hitboxes. I need this layer cause of player ghost
				

// Layer that objects can be in, determines which other objects it can collide with
// Typically you at least want to have 1 layer for moving bodies and 1 layer for static bodies, but you can have more
// layers if you want. E.g. you could have a layer for high detail collision (which is not used by the physics simulation
// but only if you do collision testing).
namespace Layers {
	static constexpr JPH::ObjectLayer NON_MOVING = 0;
	static constexpr JPH::ObjectLayer MOVING = 1;
	static constexpr JPH::ObjectLayer WALL = 2;
	static constexpr JPH::ObjectLayer ITEM = 3;
	static constexpr JPH::ObjectLayer ENEMY_HURTSPOT = 4;
	static constexpr JPH::ObjectLayer ITEM_INTERACTOR = 5;
	static constexpr JPH::ObjectLayer FLOOR = 6;
	static constexpr JPH::ObjectLayer PROPS = 7;
	static constexpr JPH::ObjectLayer PLAYERGHOST = 8;
	static constexpr JPH::ObjectLayer ATTACK_COLLIDERS = 9;
	static constexpr JPH::ObjectLayer NUM_LAYERS = 10;
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
	static constexpr JPH::BroadPhaseLayer ENEMY_HURTSPOT(4);
	static constexpr JPH::BroadPhaseLayer ITEM_INTERACTOR(5);
	static constexpr JPH::BroadPhaseLayer FLOOR(6);
	static constexpr JPH::BroadPhaseLayer PROPS(7);
	static constexpr JPH::BroadPhaseLayer PLAYERGHOST(8);
	static constexpr JPH::BroadPhaseLayer ATTACK_COLLIDERS(9);
	static constexpr JPH::uint NUM_LAYERS(10);
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
		mObjectToBroadPhase[Layers::ENEMY_HURTSPOT] = BroadPhaseLayers::ENEMY_HURTSPOT;
		mObjectToBroadPhase[Layers::ITEM_INTERACTOR] = BroadPhaseLayers::ITEM_INTERACTOR;
		mObjectToBroadPhase[Layers::FLOOR] = BroadPhaseLayers::FLOOR;
		mObjectToBroadPhase[Layers::PROPS] = BroadPhaseLayers::PROPS;
		mObjectToBroadPhase[Layers::PLAYERGHOST] = BroadPhaseLayers::PLAYERGHOST;
		mObjectToBroadPhase[Layers::ATTACK_COLLIDERS] = BroadPhaseLayers::ATTACK_COLLIDERS;
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
		case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:			return "NON_MOVING";
		case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:				return "MOVING";
		case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::WALL:				return "WALL";
		case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::ITEM:				return "ITEM";
		case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::ENEMY_HURTSPOT:		return "ENEMY_HURTSPOT";
		case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::ITEM_INTERACTOR:     return "ITEM_INTERACTOR";
		case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::FLOOR:				return "FLOOR";
		case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::PROPS:				return "PROPS";
		case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::PLAYERGHOST:		    return "PLAYERGHOST";
		case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::ATTACK_COLLIDERS:    return "ATTACK_COLLIDERS";
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
		case Layers::NON_MOVING:
			return true;
			//Enviromental Collliders
		case Layers::PROPS:
		case Layers::FLOOR:
		case Layers::WALL:
			return inLayer2 == BroadPhaseLayers::MOVING
				|| inLayer2 == BroadPhaseLayers::ITEM
				|| inLayer2 == BroadPhaseLayers::NON_MOVING
				|| inLayer2 == BroadPhaseLayers::PLAYERGHOST;
		case Layers::MOVING:
			return inLayer2 == BroadPhaseLayers::PROPS
				|| inLayer2 == BroadPhaseLayers::WALL
				|| inLayer2 == BroadPhaseLayers::FLOOR
				|| inLayer2 == BroadPhaseLayers::NON_MOVING
				|| inLayer2 == BroadPhaseLayers::MOVING
				|| inLayer2 == BroadPhaseLayers::ATTACK_COLLIDERS;	 // Moving collides with everything except item
		case Layers::ITEM:
			return inLayer2 == BroadPhaseLayers::NON_MOVING
				|| inLayer2 == BroadPhaseLayers::ITEM_INTERACTOR
				|| inLayer2 == BroadPhaseLayers::PROPS
				|| inLayer2 == BroadPhaseLayers::WALL
				|| inLayer2 == BroadPhaseLayers::FLOOR;
		case Layers::ITEM_INTERACTOR:
			return inLayer2 == BroadPhaseLayers::ITEM 
				|| inLayer2 == BroadPhaseLayers::NON_MOVING; //item interactor requires collision with items like ichor only
		case Layers::ENEMY_HURTSPOT:
			return inLayer2 == BroadPhaseLayers::NON_MOVING
				|| inLayer2 == BroadPhaseLayers::MOVING
				|| inLayer2 == BroadPhaseLayers::ENEMY_HURTSPOT;
		case Layers::PLAYERGHOST:
			return inLayer2 == BroadPhaseLayers::NON_MOVING
				|| inLayer2 == BroadPhaseLayers::PROPS
				|| inLayer2 == BroadPhaseLayers::WALL
				|| inLayer2 == BroadPhaseLayers::FLOOR
				|| inLayer2 == BroadPhaseLayers::ATTACK_COLLIDERS;
		case Layers::ATTACK_COLLIDERS:
			return inLayer2 == BroadPhaseLayers::NON_MOVING
				|| inLayer2 == BroadPhaseLayers::PROPS
				|| inLayer2 == BroadPhaseLayers::WALL
				|| inLayer2 == BroadPhaseLayers::FLOOR
				|| inLayer2 == BroadPhaseLayers::ENEMY_HURTSPOT
				|| inLayer2 == BroadPhaseLayers::MOVING
				|| inLayer2 == BroadPhaseLayers::PLAYERGHOST;
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
		case Layers::NON_MOVING:
			return true;
			//Enviromental Collliders
		case Layers::PROPS:
		case Layers::FLOOR:
		case Layers::WALL:
			return inObject2 == Layers::MOVING
				|| inObject2 == Layers::ITEM
				|| inObject2 == Layers::NON_MOVING
				|| inObject2 == Layers::PLAYERGHOST;
		case Layers::MOVING:
			return inObject2 == Layers::PROPS
				|| inObject2 == Layers::WALL
				|| inObject2 == Layers::FLOOR
				|| inObject2 == Layers::NON_MOVING
				|| inObject2 == Layers::MOVING
				|| inObject2 == Layers::ATTACK_COLLIDERS;	 // Moving collides with everything except item
		case Layers::ITEM:
			return inObject2 == Layers::NON_MOVING
				|| inObject2 == Layers::ITEM_INTERACTOR
				|| inObject2 == Layers::PROPS
				|| inObject2 == Layers::WALL
				|| inObject2 == Layers::FLOOR;
		case Layers::ITEM_INTERACTOR:
			return inObject2 == Layers::ITEM
				|| inObject2 == Layers::NON_MOVING; //item interactor requires collision with items like ichor only
		case Layers::ENEMY_HURTSPOT:
			return inObject2 == Layers::NON_MOVING
				|| inObject2 == Layers::MOVING
				|| inObject2 == Layers::ENEMY_HURTSPOT;
		case Layers::PLAYERGHOST:
			return inObject2 == Layers::NON_MOVING
				|| inObject2 == Layers::PROPS
				|| inObject2 == Layers::WALL
				|| inObject2 == Layers::FLOOR
				|| inObject2 == Layers::ATTACK_COLLIDERS;
		case Layers::ATTACK_COLLIDERS:
			return inObject2 == Layers::NON_MOVING
				|| inObject2 == Layers::PROPS
				|| inObject2 == Layers::WALL
				|| inObject2 == Layers::FLOOR
				|| inObject2 == Layers::ENEMY_HURTSPOT
				|| inObject2 == Layers::MOVING
				|| inObject2 == Layers::PLAYERGHOST;
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