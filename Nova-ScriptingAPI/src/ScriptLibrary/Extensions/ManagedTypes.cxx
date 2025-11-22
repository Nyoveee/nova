#include "ManagedTypes.hxx"
#include "nova_math.h"
#include "API/ConversionUtils.hxx"
#include "ResourceManager/resourceManager.h"
#include "ScriptLibrary.hxx"
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/quaternion.hpp>

// =================================================================
// VECTOR 2
// =================================================================

float Vector2::Length(){ return sqrt(x*x + y*y); }
void Vector2::Normalize() {
	float length = Length();
	x /= length; y /= length;
}
float Vector2::Distance(Vector2 a, Vector2 b){
	return glm::distance(a.native(), b.native());
}

// Conversion Constructor doesn't work :(
Vector2 Vector2::operator-(Vector2 a) { return Vector2{ -a.x, -a.y }; }
Vector2 Vector2::operator-(Vector2 a, Vector2 b) { return Vector2{ a.x - b.x,a.y - b.y }; }
Vector2 Vector2::operator+(Vector2 a, Vector2 b) { return Vector2{ a.x + b.x,a.y + b.y }; }
Vector2 Vector2::operator*(Vector2 a, float d) { return Vector2{ a.x * d, a.y * d}; }
Vector2 Vector2::operator*(float d, Vector2 a) { return Vector2{ a.x * d, a.y * d}; }
Vector2 Vector2::operator/(Vector2 a, float d) { return Vector2{ a.x / d, a.y / d}; }
bool Vector2::operator!=(Vector2 a, Vector2 b) { return a.native() != b.native(); }
bool Vector2::operator==(Vector2 a, Vector2 b) { return a.native() == b.native(); }
Vector2 Vector2::Up() { return Vector2{ 0.f,  1.f };}
Vector2 Vector2::Down() { return Vector2{ 0.f, -1.f };}
Vector2 Vector2::Left() { return Vector2{ -1.f,  0.f };}
Vector2 Vector2::Right() { return Vector2{ 1.f,  0.f };}
Vector2 Vector2::Zero() { return Vector2{ 0.f,  0.f };}
Vector2 Vector2::One() { return Vector2{ 1.f,  1.f };}
float Vector2::Dot(Vector2 a, Vector2 b) { return a.x * b.x + a.y * b.y; }

Vector2 Vector2::Lerp(Vector2 a, Vector2 b, float interval) {
	return Vector2{ std::lerp(a.x, b.x, interval), std::lerp(a.y, b.y, interval) };
}

// =================================================================
// VECTOR 3
// =================================================================

float Vector3::Length(){ return sqrt(x * x + y * y + z*z); }
void Vector3::Normalize() {
	float length = Length();
	
	if (length == 0) {
		return;
	}

	x /= length; y /= length; z /= length;
}
float Vector3::Distance(Vector3 a, Vector3 b){
	return glm::distance(a.native(), b.native());
}
// Conversion Constructor doesn't work :(
Vector3 Vector3::operator-(Vector3 a, Vector3 b) { return Vector3{ a.x - b.x,a.y - b.y,a.z - b.z };}
Vector3 Vector3::operator-(Vector3 a)			 { return Vector3{ -a.x, -a.y, -a.z }; }
Vector3 Vector3::operator+(Vector3 a, Vector3 b) { return Vector3{ a.x + b.x,a.y + b.y,a.z + b.z }; }
Vector3 Vector3::operator*(Vector3 a, float d)	 { return Vector3{ a.x * d, a.y * d,a.z * d }; }
Vector3 Vector3::operator*(float d, Vector3 a)   { return Vector3{ a.x * d, a.y * d,a.z * d }; }
Vector3 Vector3::operator/(Vector3 a, float d)   { return Vector3{ a.x / d, a.y / d,a.z / d }; }
bool Vector3::operator!=(Vector3 a, Vector3 b)   { return a.native() != b.native(); }
bool Vector3::operator==(Vector3 a, Vector3 b)   { return a.native() == b.native();}
Vector3 Vector3::Up(){ return Vector3{ 0.f,  1.f,  0.f };}
Vector3 Vector3::Down(){ return Vector3{ 0.f, -1.f,  0.f };}
Vector3 Vector3::Front(){ return Vector3{ 0.f,  0.f,  1.f };}
Vector3 Vector3::Back(){ return  Vector3{ 0.f,  0.f,  -1.f };}
Vector3 Vector3::Left(){ return Vector3{ -1.f,  0.f,  0.f };}
Vector3 Vector3::Right(){ return Vector3{ 1.f,  0.f,  0.f };}
Vector3 Vector3::Zero() { return Vector3{ 0.f,  0.f,  0.f };}
Vector3 Vector3::One(){ return Vector3{ 1.f,  1.f,  1.f }; }
float Vector3::Dot(Vector3 a, Vector3 b){ return a.x * b.x + a.y * b.y + a.z * b.z; }

Vector3 Vector3::Lerp(Vector3 a, Vector3 b, float interval) {
	return Vector3{ std::lerp(a.x, b.x, interval), std::lerp(a.y, b.y, interval), std::lerp(a.z, b.z, interval) };
}

// =================================================================
// QUATERNION
// =================================================================

Vector3 Quaternion::operator*(Quaternion quaternion, Vector3 axis) {
	return Vector3{ quaternion.native() * axis.native() };
}

Vector3 Quaternion::operator*(Vector3 axis, Quaternion quaternion) {
	return Vector3{ axis.native() * quaternion.native() };
}

Quaternion Quaternion::Identity() {
	return Quaternion{ glm::identity<glm::quat>() };
}

// =================================================================
// TRANSFORM
// =================================================================

void Transform_::rotate(Vector3 axis, float degrees) {
	Transform* transform = nativeComponent();

	if (transform) {
		transform->rotation = glm::rotate(transform->rotation, static_cast<float>(toRadian(degrees)), axis.native());
	}
}

void Transform_::rotate(Quaternion quartenion) {
	nativeComponent()->rotation = quartenion.native() * nativeComponent()->rotation;
}

Quaternion Transform_::LookAt(Transform_^ target) {
	Vector3 direction = position - target->position;
	direction.Normalize();
	
	return Quaternion{ glm::quatLookAt(direction.native(), glm::vec3{0,1,0}) };
}

void Transform_::setFront(Vector3 frontAxis) {
	nativeComponent()->rotation = glm::quatLookAt(-frontAxis.native(), glm::vec3{ 0,1,0 });
}

// =================================================================
// Particle Emitter
// =================================================================

void ParticleEmitter_::emit(int count)
{
	Transform* transform = Convert(gameObject->transform);
	ParticleEmitter* emitter = nativeComponent();
	if(transform && emitter)
		Interface::engine->particleSystem.emit(*transform, *emitter, count);
}

// =================================================================
// Rigidbody
// =================================================================

void Rigidbody_::AddForce(Vector3 forceVector) {
	Rigidbody* rigidbody = nativeComponent();

	if (rigidbody) {
		Interface::engine->physicsManager.addForce(*rigidbody, forceVector.native());
	}
}

void Rigidbody_::AddImpulse(Vector3 forceVector) {
	Rigidbody* rigidbody = nativeComponent();

	if (rigidbody) {
		Interface::engine->physicsManager.addImpulse(*rigidbody, forceVector.native());
	}
}

void Rigidbody_::AddVelocity(Vector3 velocity) {
	Rigidbody* rigidbody = nativeComponent();

	if (rigidbody) {
		Interface::engine->physicsManager.setVelocity(*rigidbody, rigidbody->velocity + velocity.native());
	}
}

void Rigidbody_::SetVelocity(Vector3 velocity) {
	Rigidbody* rigidbody = nativeComponent();

	if (rigidbody) {
		Interface::engine->physicsManager.setVelocity(*rigidbody, velocity.native());
	}
}

Vector3 Rigidbody_::GetVelocity() {
	Rigidbody const* rigidbody = nativeComponent();

	if (rigidbody) {
		return Vector3{ rigidbody->velocity };
	}

	return Vector3{};
}

// =================================================================
// Animator
// =================================================================

void Animator_::SetBool(System::String^ name, bool value){
	Interface::engine->animationSystem.setParameter(*nativeComponent(), Convert(name), value);
}

void Animator_::SetFloat(System::String^ name, float value){
	Interface::engine->animationSystem.setParameter(*nativeComponent(), Convert(name), value);
}

void Animator_::SetInteger(System::String^ name, int value){
	Interface::engine->animationSystem.setParameter(*nativeComponent(), Convert(name), value);
}

void Animator_::PlayAnimation(System::String^ name) {
	Interface::engine->animationSystem.playAnimation(*nativeComponent(), Convert(name));
}
// =================================================================
// Text
// =================================================================
void Text_::SetText(System::String^ text) {
	nativeComponent()->text = Convert(text);
}

// =================================================================
// Mesh Renderer & Skinned Mesh Renderer..
// =================================================================
void MeshRenderer_::changeMaterial(int index, ScriptingAPI::Material^ material) {
	MeshRenderer* meshRenderer = nativeComponent();

	if (!meshRenderer || !material) {
		return;
	}

	if (index < 0 || index >= meshRenderer->materialIds.size()) {
		Logger::warn("Invalid material index");
		return;
	}

	meshRenderer->materialIds[index] = material->getId();
}

// private helper function.. (nullable)
template <typename T>
void SetUniformValue(int index, System::String^ name, std::vector<TypedResourceID<Material>>& materialIds, std::unordered_set<int>& isMaterialInstanced, T const& data) {
	if (index < 0 || index >= materialIds.size()) {
		return;
	}

	ResourceID materialId = materialIds[index];

	if (!isMaterialInstanced.contains(index)) {
		isMaterialInstanced.insert(index);

		// We need to create a new material instance in memory..
		materialId = Interface::engine->resourceManager.createResourceInstance<Material>(materialId);
		materialIds[index] = TypedResourceID<Material>{ materialId };
	}

	auto&& [material, _] = Interface::engine->resourceManager.getResource<Material>(materialId);

	if (!material) {
		Logger::warn("Invalid material when setting material property..");
		return;
	}

	std::string parameterName = Convert(name);
	auto iterator = material->materialData.overridenUniforms.find(parameterName);

	if (iterator == material->materialData.overridenUniforms.end()) {
		Logger::warn("Invalid parameter name {} when setting material property", parameterName);
		return;
	}

	auto&& [__, uniformData] = *iterator;
	uniformData.value = data;
}

void MeshRenderer_::setMaterialFloat(int index, System::String^ name, float data) {
	SetUniformValue(index, name, nativeComponent()->materialIds, nativeComponent()->isMaterialInstanced, data);
}

void MeshRenderer_::setMaterialVector2(int index, System::String^ name, Vector2^ data) {
	SetUniformValue(index, name, nativeComponent()->materialIds, nativeComponent()->isMaterialInstanced, data->native());
}

void MeshRenderer_::setMaterialVector3(int index, System::String^ name, Vector3^ data) {
	SetUniformValue(index, name, nativeComponent()->materialIds, nativeComponent()->isMaterialInstanced, data->native());
}

void MeshRenderer_::setMaterialBool(int index, System::String^ name, bool data) {
	SetUniformValue(index, name, nativeComponent()->materialIds, nativeComponent()->isMaterialInstanced, data);
}

void MeshRenderer_::setMaterialInt(int index, System::String^ name, int data) {
	SetUniformValue(index, name, nativeComponent()->materialIds, nativeComponent()->isMaterialInstanced, data);
}

void MeshRenderer_::setMaterialUInt(int index, System::String^ name, unsigned data) {
	SetUniformValue(index, name, nativeComponent()->materialIds, nativeComponent()->isMaterialInstanced, data);
}

void SkinnedMeshRenderer_::changeMaterial(int index, ScriptingAPI::Material^ material) {
	SkinnedMeshRenderer* meshRenderer = nativeComponent();

	if (!meshRenderer || !material) {
		return;
	}

	if (index < 0 || index >= meshRenderer->materialIds.size()) {
		Logger::warn("Invalid material index");
		return;
	}

	meshRenderer->materialIds[index] = material->getId();
}

void SkinnedMeshRenderer_::setMaterialFloat(int index, System::String^ name, float data) {
	SetUniformValue(index, name, nativeComponent()->materialIds, nativeComponent()->isMaterialInstanced, data);
}

void SkinnedMeshRenderer_::setMaterialVector2(int index, System::String^ name, Vector2^ data) {
	SetUniformValue(index, name, nativeComponent()->materialIds, nativeComponent()->isMaterialInstanced, data->native());
}

void SkinnedMeshRenderer_::setMaterialVector3(int index, System::String^ name, Vector3^ data) {
	SetUniformValue(index, name, nativeComponent()->materialIds, nativeComponent()->isMaterialInstanced, data->native());
}

void SkinnedMeshRenderer_::setMaterialBool(int index, System::String^ name, bool data) {
	SetUniformValue(index, name, nativeComponent()->materialIds, nativeComponent()->isMaterialInstanced, data);
}

void SkinnedMeshRenderer_::setMaterialInt(int index, System::String^ name, int data) {
	SetUniformValue(index, name, nativeComponent()->materialIds, nativeComponent()->isMaterialInstanced, data);
}

void SkinnedMeshRenderer_::setMaterialUInt(int index, System::String^ name, unsigned data) {
	SetUniformValue(index, name, nativeComponent()->materialIds, nativeComponent()->isMaterialInstanced, data);
}
