#include "nova_math.h"

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
}

