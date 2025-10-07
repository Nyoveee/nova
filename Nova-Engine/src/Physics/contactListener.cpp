#include "contactListener.h"
#include "physicsManager.h"

NovaContactListener::NovaContactListener(PhysicsManager& physicsManager) :
	physicsManager { physicsManager }
{}

JPH::ValidateResult	NovaContactListener::OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult) {
	//std::cout << "Contact validate callback" << std::endl;

	// Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
	return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
}

void NovaContactListener::OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) {
	entt::entity entityOne = static_cast<entt::entity>(static_cast<unsigned>(inBody1.GetUserData()));
	entt::entity entityTwo = static_cast<entt::entity>(static_cast<unsigned>(inBody2.GetUserData()));

	physicsManager.submitCollision(entityOne, entityTwo);
}

void NovaContactListener::OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) {
	//std::cout << "A contact was persisted" << std::endl;
}

void NovaContactListener::OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) {}