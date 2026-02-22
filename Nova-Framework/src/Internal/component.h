#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#pragma warning(disable : 4324)			// disable warning about structure being padded, that's exactly what i wanted.

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
#include "systemResource.h"
#include "frustum.h"

// Forward declaring.
class Prefab;
class Model;
class Texture;
class EquirectangularMap;
class ScriptAsset;
class Audio;
class Material;
class Sequencer;
class Video;

// Make sure your components are of aggregate type!!
// This means it extremely easy for systems to work with these components
// Components should only hold data! Let systems work on these components.

// List all the component types. This is used as a variadic argument to certain functions.
#define ALL_COMPONENTS \
	EntityData, Transform, Light, MeshRenderer, Rigidbody, BoxCollider, SphereCollider, CapsuleCollider, MeshCollider, SkyBox, AudioComponent, PositionalAudio, Scripts,   \
	NavMeshModifier, CameraComponent, NavMeshSurface, NavMeshAgent, ParticleEmitter, Text, SkinnedMeshRenderer, Animator, \
	Image, Sequence, Button, Canvas, NavMeshOffLinks, SkyboxCubeMap, ReflectionProbe, Fog, VideoPlayer

using ScriptName   = std::string;
using LayerID	   = int;
using ComponentID  = size_t;
using PrefabEntityID = entt::entity;
using MeshIndex	= int;

#include "serializedField.h"

constexpr int NO_SHADOW_MAP = -1;
constexpr int NOT_LOADED = -1;

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
	BoneIndex attachedSocket											= NO_BONE;
	std::vector<entt::entity> children									{};
	LayerID layerId														{};

	bool isActive														= true;

	TypedResourceID<Prefab> prefabID									{ INVALID_RESOURCE_ID };
	EntityGUID entityGUID												{ Math::getGUID() };

	std::unordered_map<size_t, std::vector<int>> overridenProperties	{};
	std::unordered_map<size_t, bool> overridenComponents				{};
	std::unordered_set<ComponentID> inactiveComponents                  {};

	REFLECTABLE(
		name,
		tag,
		parent,
		attachedSocket,
		children,
		layerId,
		isActive,
		prefabID,
		entityGUID,
		overridenProperties,
		overridenComponents,
		inactiveComponents
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

	// ====== These data members are calculated by the systems and do not need to be serialised. =======
	glm::vec3 up{};
	glm::vec3 right{};
	glm::vec3 front{};

	glm::mat4x4 modelMatrix		{};	// model matrix represents the final matrix to change a object to world space.
	glm::mat3x3 normalMatrix	{};	// normal matrix is used to transform normals.
	glm::mat4x4 localMatrix		{};	// transformation matrix in respect to parent!

	glm::mat4x4 lastModelMatrix	{}; // records the previous frame's model matrix. (used for TAA).
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

	bool worldModifiedByScripting = false;

	glm::vec3 deltaPosition	{};
	glm::vec3 deltaScale	{};
	glm::quat deltaRotation {};

	bool inCameraFrustum = true;

	// not every transform will have bounding box.
	// This is used for frustum culling.
	AABB boundingBox = {};
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
	float radius			= 50.f;

	bool shadowCaster		= false;
	float orthogonalShadowCasterSize = 35.0f;
	float shadowNearPlane	= 0.1f;
	float shadowFarPlane	= 750.0f;

	REFLECTABLE(
		type,
		color,
		intensity,
		cutOffAngle,
		outerCutOffAngle,
		attenuation,
		radius,
		shadowCaster,
		orthogonalShadowCasterSize,
		shadowNearPlane,
		shadowFarPlane
	)

	// runtime variable..
	int shadowMapIndex = NO_SHADOW_MAP;		// every light component has an index to it's own array of shadow maps..
};

struct MeshRenderer {
	TypedResourceID<Model>					modelId		{ SPHERE_MODEL_ID };
	std::vector<TypedResourceID<Material>>	materialIds	{ { DEFAULT_PBR_MATERIAL_ID } };

	bool castShadow = false;
	bool shadowCullFrontFace = true;

	// std::vector<>
	REFLECTABLE(
		modelId,
		materialIds,
		castShadow,
		shadowCullFrontFace
	)

	std::unordered_set<int>					isMaterialInstanced;
};

struct SkinnedMeshRenderer {
	TypedResourceID<Model>						modelId				{ INVALID_RESOURCE_ID };
	std::vector<TypedResourceID<Material>>		materialIds			{};
	std::unordered_map<BoneIndex, entt::entity> socketConnections	{};

	bool castShadow = true;
	bool shadowCullFrontFace = true;

	REFLECTABLE(
		modelId,
		materialIds,
		socketConnections,
		castShadow,
		shadowCullFrontFace
	)

	std::unordered_set<int>					isMaterialInstanced;


	// owns all the bone's final matrices.
	// we have two copies for double buffering.. (we keep a copy of the old one)
	std::array<
		std::vector<glm::mat4x4>, 2
	> bonesFinalMatrices;

	int currentBoneMatrixIndex = 0;
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

	float speedMultiplier = 1.f;
	bool toLoop = false;
	bool startPlaying = false;

	REFLECTABLE(
		sequencerId,
		speedMultiplier,
		toLoop,
		startPlaying
	)

	float timeElapsed = 0.f;
	float lastTimeElapsed = 0.f;
	
	int currentFrame = 0;
	bool isPlaying = true;

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
		Item,
		Enemy_HurtSpot,
		Item_Interactor,
		Floor,
		Props,
		PlayerGhost,
		Attack_Colliders,
	} layer							= Layer::NonMoving;

	enum class MotionQuality {
		Discrete,
		Continuous
	} motionQuality					= MotionQuality::Discrete;

	glm::vec3 initialVelocity		{};
	float mass						{ 10.f };
	float gravityMultiplier			{ 1.f };
	NormalizedFloat restitution		{ 0.0f };

	bool isRotatable				{ true };
	bool isTrigger					{ false };
	bool dynamicCollider			{ false };
	bool toScaleWithTransform		{ true };

	REFLECTABLE(
		motionType,
		layer,
		motionQuality,
		initialVelocity,
		mass,
		gravityMultiplier,
		restitution,
		isRotatable,
		isTrigger,
		dynamicCollider,
		toScaleWithTransform
	)

	// RUNTIME PROPERTIES!
	JPH::BodyID bodyId				{}; // default constructed body ID is invalid.
	glm::vec3   velocity;
	glm::vec3   angularVelocity;

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
	TypedResourceID<EquirectangularMap> equirectangularMap{ INVALID_RESOURCE_ID };
	
	REFLECTABLE(
		equirectangularMap
	)
};

struct SkyboxCubeMap {
	TypedResourceID<CubeMap> cubeMapId{ INVALID_RESOURCE_ID };

	REFLECTABLE(
		cubeMapId
	)
};

struct Image {
	TypedResourceID<Texture>	texture		{ NONE_TEXTURE_ID };
	ColorA						colorTint	{ 1.f, 1.f, 1.f, 1.f };
	
	enum class AnchorMode {
		Center,
		BottomLeft,
		BottomRight,
		TopLeft,
		TopRight
	} anchorMode = AnchorMode::Center;

	bool toFlip = false;
	
	glm::vec2 textureCoordinatesRange { 1.f, 1.f };
	bool toTile = false;

	REFLECTABLE(
		texture,
		colorTint,
		anchorMode,
		toFlip,
		textureCoordinatesRange,
		toTile
	)
};

struct ScriptData
{
	TypedResourceID<ScriptAsset> scriptId	{ INVALID_RESOURCE_ID };
	std::vector<FieldData> fields			{};

	REFLECTABLE (
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
	float volume{ 1.f };
	bool loop{};
	enum class AudioGroup {
		SFX,
		BGM
	}audioGroup = AudioGroup::SFX;
	REFLECTABLE(
		volume,
		loop,
		audioGroup
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

	Degree fov = { 45.f };

	float nearPlane = 0.3f;
	float farPlane = 1000.f;

	REFLECTABLE(
		camStatus,
		fov,
		nearPlane,
		farPlane
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


struct NavMeshOffLinks
{
	std::string agentName;
	glm::vec3 startPoint{};
	glm::vec3 endPoint{};
	float radius = 0.5f;
	bool isBiDirectional = true;


	REFLECTABLE
	(
		agentName,
		startPoint,
		endPoint,
		radius,
		isBiDirectional
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
	
	bool updateRotation				= true; //should agent update rotation. 
	bool updatePosition             = true; //should agent update position. 
	bool autoTraverseOffMeshLink	= true;

	bool isOnOffMeshLink			= false;


	REFLECTABLE
	(
		agentName,
		agentMaxSpeed,
		agentMaxAcceleration,
		agentRotationSpeed,
		separationWeight
	)
	//Runtime variables
	int   agentIndex		= -1; // -1 means its unsued //query the mapper to find the correct dtCrowd ID. 
								  //NOTE this no longer maps directly to dtCrowd object, use GetDTCrowdIndex to find actual index
	float agentRadius	= 0.f;
	float agentHeight	= 0.f;
	navMeshOfflinkData currentData;
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
	ColorA color{ 1.f, 1.f, 1.f,1.f };
	glm::vec3 colorOffsetMin{};
	glm::vec3 colorOffsetMax{};
	float emissiveMultiplier = 1.f;
	REFLECTABLE(
		color,
		colorOffsetMin,
		colorOffsetMax,
		emissiveMultiplier
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
	glm::vec3 trailColorOffsetMin{};
	glm::vec3 trailColorOffsetMax{};
	float trailEmissiveMultiplier{ 1.f };
	REFLECTABLE(
		selected,
		trailTexture,
		distancePerEmission,
		trailSize,
		trailColor,
		trailColorOffsetMin,
		trailColorOffsetMax,
		trailEmissiveMultiplier
	)
};

struct ParticleEmitter
{
	// Update
	float currentContinuousTime{};
	float currentBurstTime{};
	glm::vec3 prevPosition{};
	bool b_firstPositionUpdate{ true };
	// Categories
	TypedResourceID<Texture> texture{};
	ParticleEmissionTypeSelection particleEmissionTypeSelection{};
	ParticleColorSelection particleColorSelection{};
	SizeOverLifetime sizeOverLifetime{};
	ColorOverLifetime colorOverLifetime{};
	Trails trails{};
	// Core
	bool looping = true;
	bool randomizedDirection = false;
	bool invertMovement = false;
	float startSize = 1.f;
	float minStartSizeOffset = 0.f;
	float maxStartSizeOffset = 0.f;
	float startSpeed = 1;
	glm::vec3 force{};
	// Velocity
	float initialAngularVelocity{};
	float minAngularVelocityOffset{};
	float maxAngularVelocityOffset{};
	// Particle spawning info
	float lifeTime = 1;
	float particleRate = 100;
	float burstRate = 0;
	int burstAmount = 30;
	// Light
	float lightIntensity{};
	glm::vec3 lightattenuation = glm::vec3{ 1.f, 0.09f, 0.032f };
	float lightRadius{};

	REFLECTABLE
	(
		texture,
		startSize,
		minStartSizeOffset,
		maxStartSizeOffset,
		startSpeed,
		initialAngularVelocity,
		minAngularVelocityOffset,
		maxAngularVelocityOffset,
		force,
		lifeTime,
		particleRate,
		burstRate,
		burstAmount,
		lightIntensity,
		lightattenuation,
		lightRadius,
		looping,
		randomizedDirection,
		invertMovement,
		particleEmissionTypeSelection,
		particleColorSelection,
		sizeOverLifetime,
		colorOverLifetime,
		trails
	)
};
struct alignas(16) ParticleLifespanData {
	glm::vec4 startColor{};
	glm::vec4 endColor{};
	alignas(16) glm::vec3 velocity{};
	alignas(16) glm::vec3 force{};
	alignas(16) glm::vec3 lightattenuation{};
	float colorInterpolation{};
	float sizeInterpolation{};
	float lightIntensity{};
	float lightRadius{};
	float angularVelocity{};
	float startSize{};
	float endSize{};
	float currentLifeTime{};
	float lifeTime{};
	int colorOverLifetime{};
	int sizeOverLifetime{};
	int b_Active{};
};

struct Text {
	TypedResourceID<Font> font;
	int fontSize = 13;
	std::string text;
	Color fontColor = Color{ 1.f, 1.f, 1.f };

	enum class Alignment {
		LeftAligned,
		Center,
		RightAligned,
	} alignment = Alignment::LeftAligned;

	REFLECTABLE
	(
		font,
		text,
		fontSize,
		fontColor,
		alignment
	)
};

struct Canvas {
	std::string placeholder;

	REFLECTABLE
	(
		placeholder
	)
};

struct EntityScript {
	entt::entity entity;
	TypedResourceID<ScriptAsset> script;
	
	REFLECTABLE(
		entity,
		script
	)
};

struct Button {
	bool isInteractable;
	EntityScript reference;

	ColorA normalColor		= ColorA{ 1.f, 1.f, 1.f, 1.f };
	ColorA highlightedColor = ColorA{ 1.f, 1.f, 1.f, 1.f };
	ColorA pressedColor		= ColorA{ 1.f, 1.f, 1.f, 1.f };
	ColorA disabledColor	= ColorA{ 1.f, 1.f, 1.f, 1.f };

	std::string onClickReleasedFunction;
	std::string onPressFunction;
	std::string onHoverFunction;

	float fadeDuration = 0.1f;
	float colorMultiplier = 1.f;

	glm::vec3 offset		= glm::vec3{ 0.f, 0.f, 0.f };
	glm::vec3 padding		= glm::vec3{ 0.f, 0.f, 0.f };

	REFLECTABLE
	(
		isInteractable,
		reference,
		normalColor,
		highlightedColor,
		pressedColor,
		disabledColor,
		onClickReleasedFunction,
		onPressFunction,
		onHoverFunction,
		fadeDuration,
		colorMultiplier,
		offset,
		padding
	)

	enum class State {
		Normal,
		Hovered,
		Pressed,
		Disabled
	} state = State::Normal;

	float timeElapsed = 0.f;

	ColorA finalColor = normalColor;

	void enableButton() {
		isInteractable = true;
		state = Button::State::Normal;
	}

	void disableButton() {
		isInteractable = false;
		state = Button::State::Disabled;
	}
};

struct ReflectionProbe {
	glm::vec3 boxExtents			= { 10.f, 10.f, 10.f };
	glm::vec3 centerOffset			= {};

	float captureRadius				= 20.f;
	
	bool toCaptureShadow			= true;
	bool toCaptureEnvironmentLight	= false;
	float fallOff					= 0.2f;
	float intensity					= 1.0f;

	TypedResourceID<CubeMap> irradianceMap				= { INVALID_RESOURCE_ID };
	TypedResourceID<CubeMap> prefilteredEnvironmentMap	= { INVALID_RESOURCE_ID };

	REFLECTABLE(
		boxExtents,
		centerOffset,
		captureRadius,
		toCaptureShadow,
		toCaptureEnvironmentLight,
		fallOff,
		intensity,
		irradianceMap,
		prefilteredEnvironmentMap
	)

	int indexToCubeMapArray = NOT_LOADED;
};

struct Fog {
	float absorptionDensity					= 0.05f;
	float inscatteringDensity				= 0.5f;
	NormalizedFloat scatteringDistribution	= 0.5f;
	NormalizedFloat heightFallOff			= 1.f;
	Color fogInscatteringColor				= Color { 1.f, 1.f, 1.f };

	float startDistance						= 1.f;
	float endDistance						= 150.f;
	float rayMarchingStepSize				= 1.0f;

	REFLECTABLE(
		absorptionDensity,
		inscatteringDensity,
		scatteringDistribution,
		heightFallOff,
		fogInscatteringColor,
		startDistance,
		endDistance,
		rayMarchingStepSize
	)
};
struct VideoPlayer {
	// Update
	float timeAccumulator{};
	bool isPlaying = false;

	// Data
	TypedResourceID<Video> videoId { INVALID_RESOURCE_ID };
	bool playOnStart = true;
	bool loop = false;
	REFLECTABLE(
		videoId,
		playOnStart,
		loop
	)
};