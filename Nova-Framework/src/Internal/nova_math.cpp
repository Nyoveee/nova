#include "nova_math.h"

#include "component.h"
#include "xresource_guid.h"

// Reference: https://stackoverflow.com/questions/17918033/glm-decompose-mat4-into-translation-and-rotation
namespace Math {
	DecomposedMatrix decomposeMatrix(glm::mat4 const& m) {
		glm::vec3 pos = m[3];
		glm::vec3 scale;

		for (int i = 0; i < 3; i++)
			scale[i] = glm::length(glm::vec3(m[i]));


		const glm::mat3 rotMtx(
			glm::vec3(m[0]) / (scale[0] == 0 ? 1 : scale[0]),
			glm::vec3(m[1]) / (scale[1] == 0 ? 1 : scale[1]),
			glm::vec3(m[2]) / (scale[2] == 0 ? 1 : scale[2])
		);

		glm::quat rot = glm::quat_cast(rotMtx);

		return {
			pos,
			rot,
			scale
		};
	}

	glm::mat4 composeMatrix(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale)
	{
		glm::mat4 translation = glm::translate(glm::mat4(1.0f), position);
		glm::mat4 rotationMatrix = glm::mat4_cast(rotation);  // Converts quaternion to a rotation matrix
		glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);

		return translation * rotationMatrix * scaleMatrix;
	}

    std::size_t getGUID() {
		xresource::instance_guid guid = xresource::instance_guid::GenerateGUIDCopy();
		return guid.m_Value;
    }

	float smoothstep(float t) {
		return t * t * (3.0f - 2.0f * t);
	}

	float sinestep(float interval) {
		float sine_input = (interval * std::numbers::pi_v<float>) - (std::numbers::pi_v<float> / 2.0f);
		return (std::sin(sine_input) + 1.0f) / 2.0f;
	}

}

