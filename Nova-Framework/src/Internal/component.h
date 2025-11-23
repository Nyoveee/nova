#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <entt/entt.hpp>
#include <string>
#include <vector>
#include <glm/mat4x4.hpp>
#include <glm/mat3x3.hpp>
#include <glm/gtx/quaternion.hpp>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <optional>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/MotionType.h>
#include <Jolt/Physics/Body/BodyID.h>

#include "type_alias.h"
#include "vertex.h"
#include "reflection.h"
#include "navMesh.h"
#include "controller.h"

class Prefab;
class Model;
class Texture;
class CubeMap;
class ScriptAsset;
class Audio;
class Material;
class Sequencer;

// Make sure your components are of aggregate type!!
// This means it extremely easy for systems to work with these components
// Components should only hold data! Let systems work on these components.

// List all the component types. This is used as a variadic argument to certain functions.
#define ALL_COMPONENTS \
	EntityData, Transform, Light, MeshRenderer, Rigidbody, BoxCollider, SphereCollider, CapsuleCollider, MeshCollider, SkyBox, AudioComponent, PositionalAudio, Scripts,   \
	NavMeshModifier, CameraComponent, NavMeshSurface, NavMeshAgent, ParticleEmitter, Text, SkinnedMeshRenderer, Animator,\
	Image, Sequence

using ScriptName   = std::string;
using LayerID	   = int;


#include "serializedField.h"

enum class InterpolationType : unsigned int {
	Root,
	Linear,
	Quadractic,
	Cubic
};


// ===================================

struct EntityData {
	std::string name													{};
	std::string tag                                                     {};
	entt::entity parent													= entt::null;
	std::vector<entt::entity> children									{};
	LayerID layerId														{};
	bool isActive														= true;

	TypedResourceID<Prefab> prefabID									{ INVALID_RESOURCE_ID };
	std::unordered_map<size_t, std::vector<int>> overridenProperties	{};

	REFLECTABLE(
		name,
		tag,
		parent,
		children,
		layerId,
		isActive,
		prefabID,
		overridenProperties
	)
};

struct Transform {
	glm::vec3 position			{};
	glm::vec3 scale				{ 1.0f, 1.0f, 1.0f };
	glm::quat rotation			{ 1.0f, 0.f, 0.f, 0.f };
	EulerAngles eulerAngles		{ rotation };	// this will be derieved from quartenions

	glm::vec3	localPosition	{};
	glm::vec3	localScale		{ 1.f, 1.f, 1.f };
	glm::quat	localRotation	{ 1.0f, 0.f, 0.f, 0.f };
	EulerAngles localEulerAngles{ localRotation };	// this will be derieved from quartenions

	// ====== These data members are calculated by the systems and do not need to be serialised. =======
	glm::vec3 up{};
	glm::vec3 right{};
	glm::vec3 front{};

	glm::mat4x4 modelMatrix		{};	// model matrix represents the final matrix to change a object to world space.
	glm::mat3x3 normalMatrix	{};	// normal matrix is used to transform normals.
	glm::mat4x4 localMatrix		{};	// transformation matrix in respect to parent!

	glm::vec3 lastPosition		{ position };
	glm::vec3 lastScale			{ scale };
	glm::quat lastRotation		{ rotation };
	EulerAngles lastEulerAngles	{ rotation };

	glm::vec3 lastLocalPosition {};
	glm::vec3 lastLocalScale	{ localScale };
	glm::quat lastLocalRotation	{};
	EulerAngles lastLocalEulerAngles{ localRotation };

	// Dirty bit indicating whether we need to recalculate the model view matrix.
	// When first created set it to true.
	bool worldHasChanged = true;
	bool needsRecalculating = true;

	// Reflect these data members for level editor to display
	REFLECTABLE(
		position,
		scale,
		rotation,
		eulerAngles,
		localPosition,
		localScale,
		localRotation,
		localEulerAngles
	)
};

struct Light {
	enum class Type : unsigned int {
		PointLight = 0,
		Directional = 1,
		Spotlight = 2
	};

	Color color				= Color{ 1.f, 1.f, 1.f };
	float intensity			= 1.f;
	Type type				= Light::Type::PointLight;
	glm::vec3 attenuation	= glm::vec3{ 1.f, 0.09f, 0.032f };
	Radian cutOffAngle		= glm::radians(12.5f);
	Radian outerCutOffAngle = glm::radians(17.5f);
	
	REFLECTABLE(
		type,
		color,
		intensity,
		cutOffAngle,
		outerCutOffAngle
	)
};

struct MeshRenderer {
	TypedResourceID<Model>					modelId		{ INVALID_RESOURCE_ID };
	std::vector<TypedResourceID<Material>>	materialIds	{};

	REFLECTABLE(
		modelId,
		materialIds
	)

	std::unordered_set<int>					isMaterialInstanced;
};

struct SkinnedMeshRenderer {
	TypedResourceID<Model>					modelId		{ INVALID_RESOURCE_ID };
	std::vector<TypedResourceID<Material>>	materialIds {};

	REFLECTABLE(
		modelId,
		materialIds
	)

	std::unordered_set<int>					isMaterialInstanced;

	// owns all the bone's final matrices.
	std::vector<glm::mat4x4> bonesFinalMatrices;
};

struct Animator {
	// @TODO: Change to animation asset.
	TypedResourceID<Controller> controllerId{ INVALID_RESOURCE_ID };

	REFLECTABLE(
		controllerId
	)

	float timeElapsed = 0;
	ControllerNodeID currentNode = NO_CONTROLLER_NODE;
	TypedResourceID<Model> currentAnimation;
	
	// this will be instantiated in runtime.
	std::vector<Controller::Parameter> parameters;

	// each animator component keeps track of already executed animation events keyframes..
	// this container is reset everytime it changes animation..
	std::unordered_set<int> executedAnimationEvents;
};

struct Sequence {
	TypedResourceID<Sequencer> sequencerId;
	bool toLoop;

	REFLECTABLE(
		sequencerId,
		toLoop
	)

	float timeElapsed = 0.f;
	float lastTimeElapsed = 0.f;
	
	int currentFrame = 0;

	// each animator component keeps track of already executed animation events keyframes..
	// this container is reset everytime it loops..
	std::unordered_set<int> executedAnimationEvents;
};

struct Rigidbody {
	JPH::EMotionType motionType		= JPH::EMotionType::Static;

	enum class Layer {
		NonMoving,
		Moving,
		Wall,
		Item
	} layer							= Layer::NonMoving;

	glm::vec3 initialVelocity		{};
	float mass						{};

	bool isRotatable				{ true };
	bool isTrigger					{ false };
	
	REFLECTABLE(
		motionType,
		layer,
		initialVelocity,
		mass,
		isRotatable,
		isTrigger
	)

	// RUNTIME PROPERTIES!
	JPH::BodyID bodyId				{}; // default constructed body ID is invalid.
	glm::vec3   velocity;

	// when this rigidbody is instantiated, this offset property is set based on whatever collider's offset.
	// runtime only and is not de/serialised.
	glm::vec3 offset;
};

struct BoxCollider {
	glm::vec3 shapeScale	{ 1.f, 1.f, 1.f };
	glm::vec3 offset		{ 0.f, 0.f, 0.f };

	bool scaleWithTransform	= true;

	REFLECTABLE(
		shapeScale,
		offset,
		scaleWithTransform
	)
};

struct SphereCollider {
	float		radius	{ 1.f };
	glm::vec3	offset	{ 0.f, 0.f, 0.f };

	REFLECTABLE(
		radius,
		offset
	)
};

struct CapsuleCollider {
	float shapeScale = 1.f;
	glm::vec3 offset{ 0.f, 0.f, 0.f };

	REFLECTABLE(
		shapeScale,
		offset
	)
};

struct MeshCollider {
	float shapeScale = 1.f;

	REFLECTABLE(
		shapeScale
	)
};

struct SkyBox {
	TypedResourceID<CubeMap> cubeMapId{ INVALID_RESOURCE_ID };
	
	REFLECTABLE(
		cubeMapId
	)
};

struct Image {
	TypedResourceID<Texture>	texture		{ INVALID_RESOURCE_ID };
	ColorA						colorTint	{ 1.f, 1.f, 1.f, 1.f };
	
	enum class AnchorMode {
		Center,
		BottomLeft,
		BottomRight,
		TopLeft,
		TopRight
	} anchorMode = AnchorMode::Center;

	REFLECTABLE(
		texture,
		colorTint,
		anchorMode
	)
};

struct ScriptData
{
	TypedResourceID<ScriptAsset> scriptId	{ INVALID_RESOURCE_ID };
	std::vector<FieldData> fields			{};

	REFLECTABLE(
		scriptId,
		fields
	)
};

struct Scripts
{
	std::vector<ScriptData> scriptDatas		{};
	REFLECTABLE(
		scriptDatas
	)
};

struct AudioData
{
	TypedResourceID<Audio> audioId			{ INVALID_RESOURCE_ID };
	float volume							{ 1.f };
	bool stopAudio							{ false };

	REFLECTABLE(
		audioId,
		volume,
		stopAudio
	)
	
};

struct AudioComponent 
{
	std::unordered_map<std::string, AudioData> data {};

	REFLECTABLE(
		data
	)
};

struct PositionalAudio
{
	bool  toggleSphere	{ false };	// toggle to see the sphere that has the radius of maxRadius
	float innerRadius	= 40.0f;	// If cameraPosition is within this inner radius, vol at max.
	float maxRadius		= 100.0f;	// If cameraPosition is within this radius, vol will change based on dist to centerPoint.

	REFLECTABLE(
		toggleSphere,
		innerRadius,
		maxRadius
	)
};

struct CameraComponent {
	bool camStatus = true;

	REFLECTABLE(
		camStatus
	)
};

struct NavMeshModifier
{
	enum class Area_Type
	{
		Walkable,
		Obstacle,
		Exclude

	} Area_Type = Area_Type::Walkable;

	REFLECTABLE
	(
		Area_Type
	)
};

struct NavMeshSurface
{
	std::string label					{};
	TypedResourceID<NavMesh> navMeshId	{ INVALID_RESOURCE_ID };

	REFLECTABLE
	(
		label,
		navMeshId
	)
};

struct NavMeshAgent
{
	//User Variables
	std::string agentName			{};
	float agentMaxSpeed				= 0.f; //Top speed
	float agentMaxAcceleration		= 0.f; //max acceleration should be abt twicse as fast as max speed
	float agentRotationSpeed		= 0.f;
	float collisionDetectionRange	= 0.f; // higher the value the earlier it attempts to steers
	float separationWeight			= 0.f;

	REFLECTABLE
	(
		agentName,
		agentMaxSpeed,
		agentMaxAcceleration,
		agentRotationSpeed,
		separationWeight
	)
	//Runtime variables
	int agentIndex		= 0;
	float agentRadius	= 0.f;
	float agentHeight	= 0.f;
};

struct NavigationTestTarget
{
	float position;

	REFLECTABLE
	(
		position
	)
};

/******************************************************************************
	Particles System
******************************************************************************/
struct Particle {
	TypedResourceID<Texture> texture;
	glm::vec3 position;
	glm::vec3 velocity;
	// Color
	glm::vec4 startColor;
	glm::vec4 currentColor;
	// Movement
	glm::vec3 direction;
	float rotation;
	// Size
	float startSize;
	float currentSize;
	// Lifetime
	float currentLifeTime;
};

struct CubeEmitter {
	glm::vec3 min = { -5.f,-5.f,-5.f };
	glm::vec3 max = { 5.f,5.f,5.f };

	REFLECTABLE(
		min,
		max
	)
};

struct ConeEmitter {
	float arc = 30.f;
	float distance = 5.f;

	REFLECTABLE(
		arc,
		distance
	)
};

struct RadiusEmitter {
	float radius = 5.f;

	REFLECTABLE(
		radius
	)
};

struct ParticleEmissionTypeSelection {
	enum class EmissionShape {
		Point,
		Sphere,
		Cube,
		Edge,
		Circle,
		Hemisphere,
		Cone
	} emissionShape = EmissionShape::Point;
	RadiusEmitter radiusEmitter;
	CubeEmitter cubeEmitter;
	ConeEmitter coneEmitter;

	REFLECTABLE(
		emissionShape,
		radiusEmitter,
		cubeEmitter,
		coneEmitter
	)
};

struct ParticleColorSelection {
	bool randomizedColor = false;
	ColorA color{ 1.f, 1.f, 1.f,1.f };

	REFLECTABLE(
		randomizedColor,
		color
	)
};

struct SizeOverLifetime {
	bool selected{};
	InterpolationType interpolationType{ InterpolationType::Linear };
	float endSize{};

	REFLECTABLE(
		selected,
		interpolationType,
		endSize
	)
};

struct ColorOverLifetime {
	bool selected{};
	InterpolationType interpolationType{ InterpolationType::Linear };
	ColorA endColor{};

	REFLECTABLE(
		selected,
		interpolationType,
		endColor
	)
};
struct Trails {
	bool selected{false};
	TypedResourceID<Texture> trailTexture;
	float distancePerEmission{0.1f};
	float trailSize{ 0.1f };
	ColorA trailColor{ ColorA{1.f,1.f,1.f,1.f} };
	REFLECTABLE(
		selected,
		trailTexture,
		distancePerEmission,
		trailSize,
		trailColor
	)
};

struct ParticleEmitter
{
	// Update
	float currentContinuousTime{};
	float currentBurstTime{};
	glm::vec3 prevPosition;
	bool b_firstPositionUpdate{ true };

	// Rendering
	std::vector<Particle> particles;
	std::vector<Particle> trailParticles;
	// Editor stuff
	TypedResourceID<Texture> texture;
	ParticleEmissionTypeSelection particleEmissionTypeSelection;
	ParticleColorSelection particleColorSelection;
	SizeOverLifetime sizeOverLifetime;
	ColorOverLifetime colorOverLifetime;
	Trails trails;
	bool looping = true;
	bool randomizedDirection = false;
	float startSize = 1;
	float startSpeed = 1;
	float angularVelocity{};
	glm::vec3 force;
	float lifeTime = 1;
	int maxParticles = 1000;
	float particleRate = 100;
	float burstRate = 0;
	int burstAmount = 30;
	float lightIntensity{};
	glm::vec3 lightattenuation = glm::vec3{ 1.f, 0.09f, 0.032f };

	REFLECTABLE
	(
		texture,
		startSize,
		startSpeed,
		angularVelocity,
		force,
		lifeTime,
		maxParticles,
		particleRate,
		burstRate,
		burstAmount,
		lightIntensity,
		lightattenuation,
		looping,
		randomizedDirection,
		particleEmissionTypeSelection,
		particleColorSelection,
		sizeOverLifetime,
		colorOverLifetime,
		trails
	)
};

struct Text {
	TypedResourceID<Font> font;
	int fontSize = 13;
	std::string text;
	Color fontColor = Color{ 0.f, 0.f, 0.f };

	REFLECTABLE
	(
		font,
		text,
		fontSize,
		fontColor
	)
};
