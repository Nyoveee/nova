#include "ManagedTypes.hxx"
#include "nova_math.h"
#include "API/ConversionUtils.hxx"
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/quaternion.hpp>
void Transform_::rotate(Vector3 axis, float degrees) {
	Transform* transform = nativeComponent();

	if (transform) {
		transform->rotation = glm::rotate(transform->rotation, static_cast<float>(toRadian(degrees)), axis.native());
	}
}

void ParticleEmitter_::emit(int count)
{
	Transform* transform = Convert(gameObject->transform);
	ParticleEmitter* emitter = nativeComponent();
	if(transform && emitter)
		Interface::engine->particleSystem.emit(*transform, *emitter, count);
}
