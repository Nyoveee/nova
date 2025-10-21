#include "ManagedTypes.hxx"
#include "nova_math.h"

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/quaternion.hpp>

void Transform_::rotate(Vector3 axis, float degrees) {
	Transform* transform = nativeComponent();

	if (transform) {
		transform->rotation = glm::rotate(transform->rotation, static_cast<float>(toRadian(degrees)), axis.native());
	}
}
