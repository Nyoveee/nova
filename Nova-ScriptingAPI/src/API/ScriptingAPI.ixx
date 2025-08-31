#include "ScriptingAPI.hxx"
template<typename T>
T* Interface::getNativeComponent(System::UInt32 entityID) {
	// Get the Entt reference from the engine
	entt::entity entity{ static_cast<entt::entity>(entityID) };
	if (!registry->any_of<T>(entity))
		return nullptr;
	return &(registry->get<T>(entity));
}