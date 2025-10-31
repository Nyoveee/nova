#include "ManagedTypes.hxx"
#include "nova_math.h"
#include "API/ConversionUtils.hxx"
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



// =================================================================
// TRANSFORM
// =================================================================

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