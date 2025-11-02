#pragma once

#include "component.h"
#include "API/ManagedTypeMacros.hxx"

// ===========================================================================================
// 1. Defining structs..
// 
// This creates a new managed type, that will be associated with the given native type. (must be an aggregate class!!)
// This is important as C# Scripts can only interact with managed types and not native types directly.
// Provide the respective data members you want your managed type to contain. Name of the data member must match
// exactly with the native type.
// 
// Providing association here also allows you to declare data members of Managed Components
// later in 2. with the managed type.
// ===========================================================================================

// ======================================
// This struct is responsible for Vector2 Types
// ======================================

ManagedStruct(
	Vector2, glm::vec2,
	float, x,
	float, y
)

float Length();
void Normalize();
static float Distance(Vector2 a, Vector2 b);
static Vector2 operator-(Vector2 a, Vector2 b);
static Vector2 operator-(Vector2 a);
static Vector2 operator+(Vector2 a, Vector2 b);
static Vector2 operator*(Vector2 a, float d);
static Vector2 operator*(float d, Vector2 a);
static Vector2 operator/(Vector2 a, float d);
static bool operator!=(Vector2 a, Vector2 b);
static bool operator==(Vector2 a, Vector2 b);
static Vector2 Up();
static Vector2 Down();
static Vector2 Left();
static Vector2 Right();
static Vector2 One();
static Vector2 Zero();

ManagedStructEnd(Vector2, glm::vec2)
// ======================================
// This struct is responsible for Vector3 Types
// ======================================
ManagedStruct(
	Vector3, glm::vec3,
	float, x,
	float, y,
	float, z
)

float Length();
void Normalize();

static float Distance(Vector3 a, Vector3 b);
static Vector3 operator-(Vector3 a, Vector3 b);
static Vector3 operator-(Vector3 a);
static Vector3 operator+(Vector3 a, Vector3 b);
static Vector3 operator*(Vector3 a, float d);
static Vector3 operator*(float d, Vector3 a);
static Vector3 operator/(Vector3 a, float d);
static bool operator!=(Vector3 a, Vector3 b);
static bool operator==(Vector3 a, Vector3 b);
static Vector3 Up();
static Vector3 Down();
static Vector3 Front();
static Vector3 Back();
static Vector3 Left();
static Vector3 Right();
static Vector3 One();
static Vector3 Zero();

ManagedStructEnd(Vector3, glm::vec3)
// ======================================
// This struct is responsible for ColorA Types
// ======================================
ManagedStruct(
	ColorAlpha, glm::vec4,
	float, r,
	float, g,
	float, b,
	float, a
)
ManagedStructEnd(ColorAlpha, glm::vec4)
// ======================================
// This struct is responsible for Quartenion Types
// ======================================
ManagedStruct(
	Quartenion, glm::quat,
	float, x,
	float, y,
	float, z,
	float, w
)

static Vector3 operator*(Quartenion quaternion, Vector3 axis);
static Vector3 operator*(Vector3 axis, Quartenion quaternion);

ManagedStructEnd(Quartenion, glm::quat)

// ======================================
// This struct is responsible for Ray Types
// ======================================
ManagedStruct(
	Ray, PhysicsRay,
	Vector3, origin,
	Vector3, direction
)
ManagedStructEnd(Ray, PhysicsRay)
// ======================================
// This struct is responsible for RayCastResult Types
// ======================================

ManagedStruct(
	RayCastResult, PhysicsRayCastResult,
	entt::entity, entity,
	Vector3, point
)

ManagedStructEnd(RayCastResult, PhysicsRayCastResult)

// ===========================================================================================
// 2. Defining managed component types..
// 
// This defines a new managed component, that will be associated with the given native component. 
// The new name of the managed component would append a underscore `XXXX_` at the back.
// 
// List down the respective data members you want the managed component to have access, along with
// the type. The name of the data member must match exactly with the identifier in the Component!
// 
// The type of the data member must match with the original data member. It must either be a primitive type, 
// or a managed type associated with the native type of the data member.
// 
// Defining a component here allows the C# script to call GetComponent<...> to retrieve the components
// of a given entity.
// ===========================================================================================



// ======================================
// Transform Component
// ======================================
ManagedComponentDeclaration(
	Transform,						// Creates a new managed component Transform_ that is associated with the Transform component
	Vector3,	position,			// Transform_ now has data member position, of type Vector3 which is associated with glm::vec3 (type of original data member).
	Vector3,	scale,
	Vector3,	front,
	Vector3,	right,
	Vector3,	up,
	Vector3,	eulerAngles,
	Vector3,	localEulerAngles,
	Quartenion, rotation,
	Quartenion, localRotation
)

void rotate(Vector3 axis, float angle);
void rotate(Quartenion quartenion);

Quartenion LookAt(Transform_^ target);
void setFront(Vector3 frontAxis);

ManagedComponentEnd()
// ======================================
// ParticleEmitter Component
// ======================================
ManagedComponentDeclaration(
	ParticleEmitter
)
void emit(int count);

ManagedComponentEnd()
// ======================================
// RigidBody Component
// ======================================
ManagedComponentDeclaration(
	Rigidbody
)
void AddForce(Vector3 forceVector);
void AddImpulse(Vector3 forceVector);

void AddVelocity(Vector3 velocity);
void SetVelocity(Vector3 velocity);
Vector3 GetVelocity();

ManagedComponentEnd()

// ======================================
// Animator Component
// ======================================
ManagedComponentDeclaration(
	Animator
)
void SetBool(System::String^ name, bool value);
void SetFloat(System::String^ name, float value);
void SetInteger(System::String^ name, int value);

void PlayAnimation(System::String^ name);

ManagedComponentEnd()
// ======================================
// Image Component
// ======================================
ManagedComponentDeclaration(
	Image,
	ColorAlpha, colorTint
)
ManagedComponentEnd()
// ======================================
// Text Component
// ======================================
ManagedComponentDeclaration(
	Text
)
void SetText(System::String^ text);
ManagedComponentEnd()