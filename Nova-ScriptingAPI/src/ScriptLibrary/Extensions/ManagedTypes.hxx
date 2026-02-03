#pragma once

#include "component.h"
#include "ECS/ECS.h"
#include "API/ManagedTypeMacros.hxx"
#include "API/IManagedResourceID.hxx"
// ===========================================================================================
// 0. Defining managed typed resource..
// ===========================================================================================

namespace ScriptingAPI {
	ManagedResource(Prefab)
	ManagedResource(Texture)
	ManagedResource(Model)
	ManagedResource(Material)
	ManagedResource(Scene)
	ManagedResource(Audio)
}


#define ALL_MANAGED_TYPED_RESOURCE_ID \
	ScriptingAPI::Prefab, ScriptingAPI::Texture, ScriptingAPI::Model, ScriptingAPI::Material, ScriptingAPI::Scene, ScriptingAPI::Audio

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
static float Dot(Vector2 a, Vector2 b); 
static Vector2 Lerp(Vector2 a, Vector2 b, float interval);


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
static float Dot(Vector3 a, Vector3 b);
static Vector3 Lerp(Vector3 a, Vector3 b, float interval);
static Vector3 Proj(Vector3 vector, Vector3 onNormal);
static Vector3 Cross(Vector3 lhs, Vector3 rhs);



ManagedStructEnd(Vector3, glm::vec3)
// ======================================
// This struct is responsible for ColorA Types
// ======================================
ManagedStruct(
	Colour, glm::vec3,
	float, r,
	float, g,
	float, b
)
ManagedStructEnd(Colour, glm::vec3)

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
	Quaternion, glm::quat,
	float, w,
	float, x,
	float, y,
	float, z
)

static Vector3 operator*(Quaternion quaternion, Vector3 axis);
static Vector3 operator*(Vector3 axis, Quaternion quaternion);
static Quaternion Identity();
static Quaternion Slerp(Quaternion a, Quaternion b, float t);
static Quaternion LookRotation(Vector3 directionToLook);
static Quaternion AngleAxis(float angle, Vector3 axis);
static Quaternion operator*(Quaternion lhs,Quaternion rhs);
ManagedStructEnd(Quaternion, glm::quat)

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
	Vector3, point,
	Vector3, hitSurfaceNormal

)

ManagedStructEnd(RayCastResult, PhysicsRayCastResult)


// ======================================
// NavMesh OffLink Data
// ======================================
ManagedStruct(
	NavMeshOfflinkData, navMeshOfflinkData,
	bool,    valid,
	Vector3, startNode,
	Vector3, endNode
)

ManagedStructEnd(NavMeshOfflinkData, navMeshOfflinkData)

// ======================================
// NavMesh Path TO DO: Later do bah, seems complicated :P
// ======================================

//ManagedStruct(
//	NavmeshPath, navMeshPath,
//)
//
//ManagedStructEnd(NavmeshPath, navMeshPath)

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
	Vector3,	localPosition,
	Vector3,	localScale,
	Quaternion, rotation,
	Quaternion, localRotation
)

void rotate(Vector3 axis, float angle);
void rotate(Quaternion quartenion);

Quaternion LookAt(Transform_^ target);
void setFront(Vector3 frontAxis);

ManagedComponentEnd()
// ======================================
// ParticleEmitter Component
// ======================================
ManagedComponentDeclaration(
	ParticleEmitter,
	float, lifeTime
)
void emit(int count);
void emit();
void setParticleColor(ColorAlpha color);

ManagedComponentEnd()
// ======================================
// RigidBody Component
// ======================================
ManagedComponentDeclaration(
	Rigidbody,
)

System::String^ GetLayerName();

void AddForce(Vector3 forceVector);
void AddImpulse(Vector3 forceVector);

void AddVelocity(Vector3 velocity);
void SetVelocity(Vector3 velocity);
void SetVelocityLimits(float maxVelocity);
System::Nullable<float> GetVelocityLimits();
Vector3 GetVelocity();


void AddAngularVelocity(Vector3 velocity);
void SetAngularVelocity(Vector3 velocity);
void SetAngularVelocityLimits(float maxAngularVelocity);
System::Nullable<float> GetAngularVelocityLimits();
Vector3 GetAngularVelocity();

//Note Damping is synonymous to Drag, acts as a total sum of energy dissipation over time. Acts like global friction :)
void SetLinearDamping(float dampingValue);
System::Nullable<float> GetLinearDamping(); //some locking operation being done by jolt physics? possibly null?
void SetAngularDamping(float dampingValue);
System::Nullable<float> GetAngularDamping(); //some locking operation being done by jolt physics? possibly null?
//tbh idk if in our use case we need to null it i am almost 99% sure it will never be null. 

void SetBodyRotation(Quaternion rotation);

void SetGravityFactor(float factor);
float GetGravityFactor();

void SetMass(float mass);
float GetMass();

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
	ColorAlpha, colorTint,
	Vector2, textureCoordinatesRange
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

// ======================================
// MeshRenderer & SkinnedMeshRenderer Component
// ======================================
ManagedComponentDeclaration(
	MeshRenderer
)

void changeMaterial(int index, ScriptingAPI::Material^ material);

// void setMaterialVector4(int index, System::String^ name, Vector4^ data);
void setMaterialVector3(int index, System::String^ name, Vector3^ data);
void setMaterialVector2(int index, System::String^ name, Vector2^ data);
void setMaterialBool(int index, System::String^ name, bool data);
void setMaterialInt(int index, System::String^ name, int data);
void setMaterialUInt(int index, System::String^ name, unsigned data);
void setMaterialFloat(int index, System::String^ name, float data);

ManagedComponentEnd()

ManagedComponentDeclaration(
	SkinnedMeshRenderer
)

void changeMaterial(int index, ScriptingAPI::Material^ material);

void setMaterialVector3(int index, System::String^ name, Vector3^ data);
void setMaterialVector2(int index, System::String^ name, Vector2^ data);
void setMaterialBool(int index, System::String^ name, bool data);
void setMaterialInt(int index, System::String^ name, int data);
void setMaterialUInt(int index, System::String^ name, unsigned data);
void setMaterialFloat(int index, System::String^ name, float data);

ManagedComponentEnd()

// ======================================
// NavMeshAgent Component
// ======================================
ManagedComponentDeclaration(
	NavMeshAgent
)

//Warp to the nearest polygon given a point is that close by
bool Warp(Vector3^ newPosition);

bool getIsUpdateRotation();
void setIsUpdateRotation(bool setValue);

bool getIsUpdatePosition();
void setIsUpdatePosition(bool setValue);

bool getAutomateNavMeshOfflinksState();
void setAutomateNavMeshOfflinksState(bool setValue);

void CompleteOffMeshLink();

bool isOnOffMeshLinks();

NavMeshOfflinkData getOffLinkData();

ManagedComponentEnd()

// ======================================
// Sequence Component
// ======================================
ManagedComponentDeclaration(
	Sequence,
	float, speedMultiplier,
	bool, toLoop
)

bool isPlaying();

void resume();
void play();
void pause();

ManagedComponentEnd()

// ======================================
// Audio Component
// ======================================
ManagedComponentDeclaration(
	Light,
	float, intensity,
	float, radius
)

ManagedComponentEnd()

#undef PlaySound
ManagedComponentDeclaration(
	AudioComponent,
	float, volume
)
void PlaySound(ScriptingAPI::Audio^ audio);
void PlayRandomSound(System::Collections::Generic::List<ScriptingAPI::Audio^>^ audio);
void PlayBGM(ScriptingAPI::Audio^ audio);
void StopSound(ScriptingAPI::Audio^ audio);

ManagedComponentEnd()
