#include "contactListener.h"
#include "physicsManager.h"

NovaContactListener::NovaContactListener(PhysicsManager& physicsManager) :
	physicsManager { physicsManager }
{}

JPH::ValidateResult	NovaContactListener::OnContactValidate([[maybe_unused]] const JPH::Body& inBody1, [[maybe_unused]] const JPH::Body& inBody2, [[maybe_unused]] JPH::RVec3Arg inBaseOffset, [[maybe_unused]] const JPH::CollideShapeResult& inCollisionResult) {
	// Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
	return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
}

void NovaContactListener::OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, [[maybe_unused]] const JPH::ContactManifold& inManifold, [[maybe_unused]] JPH::ContactSettings& ioSettings) {
	entt::entity entityOne = static_cast<entt::entity>(static_cast<unsigned>(inBody1.GetUserData()));
	entt::entity entityTwo = static_cast<entt::entity>(static_cast<unsigned>(inBody2.GetUserData()));

	physicsManager.submitCollisionEnter(entityOne, entityTwo);
}

void NovaContactListener::OnContactPersisted([[maybe_unused]] const JPH::Body& inBody1, [[maybe_unused]] const JPH::Body& inBody2, [[maybe_unused]] const JPH::ContactManifold& inManifold, [[maybe_unused]] JPH::ContactSettings& ioSettings) {
	//std::cout << "A contact was persisted" << std::endl;


}

void NovaContactListener::OnContactRemoved([[maybe_unused]] const JPH::SubShapeIDPair& inSubShapePair) {


	//We are using non locking variant here because there are issue accessing it during callback or some reason? (hangs the game) i am not exactly clear on the order of operations so....
	//might need to look into it. causing some kind of Deadlock. DEADLOCK say that again??? my favourite game :)
	JPH::BodyID id1 = inSubShapePair.GetBody1ID();
	JPH::BodyID id2 = inSubShapePair.GetBody2ID();

	entt::entity entityOne = entt::null;
	entt::entity entityTwo = entt::null;

	if (physicsManager.isBodyAlive(id1))
	{

		JPH::BodyLockRead lock1(physicsManager.physicsSystem.GetBodyLockInterfaceNoLock(), id1);

		if (lock1.Succeeded())
		{
			entityOne = static_cast<entt::entity>(static_cast<unsigned>(lock1.GetBody().GetUserData()));
		}

	}

	if (physicsManager.isBodyAlive(id2))
	{
		JPH::BodyLockRead lock2(physicsManager.physicsSystem.GetBodyLockInterfaceNoLock(), id2);

		if (lock2.Succeeded())
		{
			entityTwo = static_cast<entt::entity>(static_cast<unsigned>(lock2.GetBody().GetUserData()));
		}
	}

	//pass to submit collision exit
	physicsManager.submitCollisionExit(entityOne, entityTwo);

}