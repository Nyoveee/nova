#include "particleSystem.h"
#include "engine.h"
#include "RandomRange.h"
#include "Interpolation.h"

#include <algorithm>

#undef max

constexpr int LOCALWORKGROUPSIZE = 128;
constexpr int MAX_NUMBER_OF_LIGHT = 100;

ParticleSystem::ParticleSystem(Engine& p_engine)
	: engine{ p_engine }
	, interpolationType{}
	, particleUpdateComputeShader{ "System/Shader/ParticleSystem/particleupdate.compute" }
	, particleFindLightsComputeShader{ "System/Shader/ParticleSystem/particleFindLights.compute" }
	, particleResetAllComputeShader{ "System/Shader/ParticleSystem/particleResetAll.compute" }
	, particleVerticesBO{ static_cast<int>(getMaxParticles() * sizeof(ParticleVertex)) }
	, particlesSSBO{ static_cast<int>(getMaxParticles() * sizeof(ParticleLifespanData) + alignof(ParticleLifespanData))}
	, particleLightsSSBO{ MAX_NUMBER_OF_LIGHT * sizeof(PointLightData) + alignof(PointLightData) }
{
	// Bind the respective ssbo to index
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, particleVerticesBO.id());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, particlesSSBO.id());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, particleLightsSSBO.id());
	// Intialize buffer values
	int maxParticles{ getMaxParticles() };

	glNamedBufferSubData(particlesSSBO.id(), 0, sizeof(int), &maxParticles);
	
	// Interpolation
	interpolationType[InterpolationType::Root] = 0.5f;
	interpolationType[InterpolationType::Linear] = 1.0f;
	interpolationType[InterpolationType::Quadractic] = 2.0f;
	interpolationType[InterpolationType::Cubic] = 3.0f;
	// Intialize all particles to inactive
	reset();
}

void ParticleSystem::reset() {
	// Call the reset compute shader
	particleResetAllComputeShader.use();
	glDispatchCompute((MAX_TEXTURETYPES * MAX_PARTICLESPERTEXTURE) / LOCALWORKGROUPSIZE + 1, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	// Reset the texture array
	usedTextures.clear();
	
	counterInEachTextures.resize(MAX_TEXTURETYPES, 0);
}

void ParticleSystem::update(float dt)
{
	// Generation of Particles
	for (auto&& [entity, transform, entityData, emitter] : engine.ecs.registry.view<Transform, EntityData, ParticleEmitter>().each()){ 
		if (!entityData.isActive || !engine.ecs.isComponentActive<ParticleEmitter>(entity))
			continue;
		continuousGeneration(transform, emitter, dt);
		burstGeneration(transform, emitter, dt);
		trailGeneration(transform, emitter);
	}
	particleUpdateComputeShader.use();
	particleUpdateComputeShader.setFloat("dt", dt);
	// Update the particles over life span
	int numTextures{ static_cast<int>(usedTextures.size()) };
	if (numTextures != 0) {
		glDispatchCompute((numTextures * MAX_PARTICLESPERTEXTURE) / LOCALWORKGROUPSIZE + 1, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}
}

int ParticleSystem::getMaxParticles()
{
	return MAX_PARTICLESPERTEXTURE * MAX_TEXTURETYPES;
}

void ParticleSystem::continuousGeneration(Transform const& transform, ParticleEmitter& emitter, float dt)
{
	if (emitter.particleRate <= 0 || !emitter.looping)
		return;
	emitter.currentContinuousTime -= dt;
	while (emitter.currentContinuousTime <= 0) {
		emitter.currentContinuousTime += 1.f / emitter.particleRate;
		spawnParticle(transform, emitter);
	}
}

void ParticleSystem::burstGeneration(Transform const& transform, ParticleEmitter& emitter, float dt)
{
	if (emitter.burstRate <= 0 || !emitter.looping)
		return;
	emitter.currentBurstTime -= dt;
	while (emitter.currentBurstTime <= 0) {
		emitter.currentBurstTime += 1.f / emitter.burstRate;
		emit(transform, emitter, emitter.burstAmount);
	}
}

void ParticleSystem::trailGeneration(Transform& transform, ParticleEmitter& emitter)
{
	if (!emitter.trails.selected || emitter.trails.distancePerEmission <= 0)
		return;
	float distancePerEmission{ std::max(emitter.trails.distancePerEmission,0.00001f) }; // Hard limit to distance
	if (glm::distance(emitter.prevPosition, transform.position) > distancePerEmission) {
		if (emitter.b_firstPositionUpdate) {
			emitter.prevPosition = transform.position;
			emitter.b_firstPositionUpdate = false;
			return;
		}
		float maxDistance{ glm::distance(emitter.prevPosition,transform.position) };
		glm::vec3 startPosition{ emitter.prevPosition };
		glm::vec3 direction{ glm::normalize(transform.position - emitter.prevPosition) };
		int index{};
		while (maxDistance > 0.f) {
			ParticleLifespanData particleLifeSpanData{};
			ParticleVertex particleVertex{};
			determineParticleSpawnDetails(
				particleLifeSpanData,
				particleVertex,
				startPosition + direction * (distancePerEmission * index), 
				emitter, 
				ParticleEmissionTypeSelection::EmissionShape::Point
			);
			determineParticleColor(
				particleLifeSpanData,
				particleVertex,
				emitter,
				emitter.trails.trailEmissiveMultiplier,
				emitter.trails.trailColor,
				emitter.colorOverLifetime.endColor,
				emitter.trails.trailColorOffsetMin,
				emitter.trails.trailColorOffsetMax
			);
			determineParticleSize(
				particleLifeSpanData,
				particleVertex,
				emitter,
				emitter.trails.trailSize,
				emitter.sizeOverLifetime.endSize,
				0,
				0
			);
			// Ignore the set velocity and force
			particleLifeSpanData.velocity = glm::vec3{ 0,0,0 };
			particleLifeSpanData.force = glm::vec3{ 0,0,0 };
			addParticleToList(particleLifeSpanData, particleVertex,emitter.trails.trailTexture);
			// Update the loop
			++index;
			maxDistance -= distancePerEmission;
		}
		emitter.prevPosition = transform.position;
	}
}

void ParticleSystem::determineParticleSpawnDetails(
	ParticleLifespanData& particleLifeSpanData, 
	ParticleVertex& particleVertex,
	glm::vec3 position,
	ParticleEmitter& emitter, 
	ParticleEmissionTypeSelection::EmissionShape emissionShape)
{
	// Helper
	auto determineParticleVelocity = [](ParticleEmitter& emitter, glm::vec3 nonRandomizedDirection){
		if (!emitter.randomizedDirection)
			return nonRandomizedDirection;
		return glm::vec3(RandomRange::Float(-1, 1), RandomRange::Float(-1, 1), RandomRange::Float(-1, 1)) * emitter.startSpeed;
	};
	// Lifetime
	particleLifeSpanData.currentLifeTime = particleLifeSpanData.lifeTime = emitter.lifeTime;
	switch (emissionShape) {
		case ParticleEmissionTypeSelection::EmissionShape::Sphere:
		{
			glm::vec3 randomSpawnDirection = glm::vec3(RandomRange::Float(-1, 1), RandomRange::Float(-1, 1), RandomRange::Float(-1, 1));
			randomSpawnDirection = glm::normalize(randomSpawnDirection);
			particleVertex.position = position + randomSpawnDirection * RandomRange::Float(0, emitter.particleEmissionTypeSelection.radiusEmitter.radius);
			particleLifeSpanData.velocity = determineParticleVelocity(emitter, randomSpawnDirection * emitter.startSpeed);
			break;
		}
		case ParticleEmissionTypeSelection::EmissionShape::Point:
		{
			glm::vec3 randomVelocity = glm::vec3(RandomRange::Float(-1, 1), RandomRange::Float(-1, 1), RandomRange::Float(-1, 1));
			randomVelocity = glm::normalize(randomVelocity);
			randomVelocity *= emitter.startSpeed;
			particleVertex.position = position;
			particleLifeSpanData.velocity = randomVelocity;
			break;
		}
		case ParticleEmissionTypeSelection::EmissionShape::Cube:
		{
			glm::vec3 min{ emitter.particleEmissionTypeSelection.cubeEmitter.min }, max{ emitter.particleEmissionTypeSelection.cubeEmitter.max };
			glm::vec3 randomSpawnPoint = position + glm::vec3{ RandomRange::Float(min.x,max.x),RandomRange::Float(min.y,max.y),RandomRange::Float(min.z,max.z) };
			particleVertex.position = randomSpawnPoint;
			particleLifeSpanData.velocity = determineParticleVelocity(emitter, glm::normalize(randomSpawnPoint - position) * emitter.startSpeed);
			break;
		}
		case ParticleEmissionTypeSelection::EmissionShape::Edge:
		{
			glm::vec3 randomSpawnPoint = position;
			randomSpawnPoint -= glm::vec3{ 1,0,0 } *emitter.particleEmissionTypeSelection.radiusEmitter.radius / 2.f;
			randomSpawnPoint += glm::vec3{ 1,0,0 } *RandomRange::Float(0, emitter.particleEmissionTypeSelection.radiusEmitter.radius);
			particleVertex.position = randomSpawnPoint;
			particleLifeSpanData.velocity = determineParticleVelocity(emitter, glm::vec3{ 0,1,0 } *emitter.startSpeed);
			break;
		}
		case ParticleEmissionTypeSelection::EmissionShape::Circle:
		{
			glm::vec3 randomSpawnDirection = glm::vec3(RandomRange::Float(-1, 1), 0, RandomRange::Float(-1, 1));
			randomSpawnDirection = glm::normalize(randomSpawnDirection);
			particleVertex.position = position + randomSpawnDirection * RandomRange::Float(0, emitter.particleEmissionTypeSelection.radiusEmitter.radius);
			particleLifeSpanData.velocity = determineParticleVelocity(emitter, randomSpawnDirection * emitter.startSpeed);
			break;
		}
		case ParticleEmissionTypeSelection::EmissionShape::Hemisphere:
		{
			glm::vec3 randomSpawnDirection = glm::vec3(RandomRange::Float(-1, 1), RandomRange::Float(0, 1), RandomRange::Float(-1, 1));
			randomSpawnDirection = glm::normalize(randomSpawnDirection);
			particleVertex.position = position + randomSpawnDirection * RandomRange::Float(0, emitter.particleEmissionTypeSelection.radiusEmitter.radius);
			particleLifeSpanData.velocity = determineParticleVelocity(emitter, randomSpawnDirection * emitter.startSpeed);
			break;
		}
		case ParticleEmissionTypeSelection::EmissionShape::Cone:
		{
			// Emission Details
			RadiusEmitter& radiusEmitter{ emitter.particleEmissionTypeSelection.radiusEmitter };
			ConeEmitter& coneEmitter{ emitter.particleEmissionTypeSelection.coneEmitter };
			float radius = radiusEmitter.radius, distance = coneEmitter.distance;
			float arc = Radian{ Degree{std::clamp(coneEmitter.arc, 0.f, 75.f)} };
			float outerRadius = radius + coneEmitter.distance * std::sin(arc);
			// Other Details
			float spawnRadius = RandomRange::Float(0, radius);
			// Calculate the spawn to target Position
			glm::vec3 randomSpawnDirection = glm::vec3(RandomRange::Float(-1, 1), 0, RandomRange::Float(-1, 1));
			randomSpawnDirection = glm::normalize(randomSpawnDirection);
			glm::vec3 spawnPosition = position + randomSpawnDirection * spawnRadius;
			glm::vec3 targetPosition = position + glm::vec3{ 0,distance,0 } + randomSpawnDirection * spawnRadius / radius * outerRadius;
			glm::vec3 velocity = glm::normalize(targetPosition - spawnPosition) * emitter.startSpeed;
			// Set the new Particle details
			particleVertex.position = spawnPosition;
			particleLifeSpanData.velocity = determineParticleVelocity(emitter, velocity);
			break;
		}
	}
	// Other Particle Details
	particleLifeSpanData.force = emitter.force;
	particleLifeSpanData.angularVelocity = emitter.initialAngularVelocity + RandomRange::Float(emitter.minAngularVelocityOffset, emitter.maxAngularVelocityOffset);
	particleLifeSpanData.lightIntensity = emitter.lightIntensity;
	particleLifeSpanData.lightattenuation = emitter.lightattenuation;
	particleLifeSpanData.lightRadius = emitter.lightRadius;
}

void ParticleSystem::determineParticleColor(
	ParticleLifespanData& particleLifeSpanData, 
	ParticleVertex& particleVertex,
	ParticleEmitter& emitter,
	float emissiveMultiplier,
	ColorA startcolor, ColorA endColor,
	glm::vec3 colorOffsetMin, glm::vec3 colorOffsetMax)
{
	// Color
	particleLifeSpanData.startColor = particleVertex.color = startcolor;
	glm::vec4 color = emitter.particleColorSelection.color;
	ColorA startColor = color + glm::vec4(glm::vec3(RandomRange::Vec3(colorOffsetMin, colorOffsetMax)), 0);
	startColor = glm::vec4(startColor.r(), startColor.g(), startColor.b(), 0) * emissiveMultiplier + glm::vec4(0, 0, 0, startColor.a());
	particleLifeSpanData.colorInterpolation = interpolationType[emitter.colorOverLifetime.interpolationType];
	particleLifeSpanData.startColor = particleVertex.color = startColor;
	particleLifeSpanData.colorOverLifetime = emitter.colorOverLifetime.selected;
	particleLifeSpanData.endColor = endColor;
}

void ParticleSystem::determineParticleSize(
	ParticleLifespanData& particleLifeSpanData,
	ParticleVertex& particleVertex,
	ParticleEmitter& emitter,
	float startSize, float endSize,
	float minStartSizeOffset, float maxStartSizeOffset)
{
	particleLifeSpanData.sizeOverLifetime = emitter.sizeOverLifetime.selected;
	particleLifeSpanData.sizeInterpolation = interpolationType[emitter.sizeOverLifetime.interpolationType];
	particleLifeSpanData.startSize = particleVertex.currentSize = startSize + RandomRange::Float(minStartSizeOffset, maxStartSizeOffset);
	particleLifeSpanData.endSize = endSize;
}

void ParticleSystem::rotateParticle(ParticleLifespanData& particleLifeSpanData, ParticleVertex& particleVertex, Transform const& transform)
{
	// Position
	glm::mat4 model = glm::identity<glm::mat4>();
	model = glm::translate(model, -transform.position);
	model = glm::mat4_cast(transform.rotation) * model;
	glm::vec4 rotatedPosFromOrigin = glm::vec4(particleVertex.position, 1.0);
	rotatedPosFromOrigin = model * rotatedPosFromOrigin;
	particleVertex.position = glm::vec3{ rotatedPosFromOrigin.x,rotatedPosFromOrigin.y,rotatedPosFromOrigin.z } + transform.position;
	// Velocity
	glm::vec4 rotatedVelocity = glm::vec4(particleLifeSpanData.velocity, 1.0);
	rotatedVelocity = glm::mat4_cast(transform.rotation) * rotatedVelocity;
	particleLifeSpanData.velocity = glm::vec3{ rotatedVelocity.x,rotatedVelocity.y,rotatedVelocity.z };
	// Force
	glm::mat4 rotation = glm::mat4_cast(transform.rotation) * glm::identity<glm::mat4>();
	glm::vec4 rotatedForce = rotation * glm::vec4{ particleLifeSpanData.force, 0 };
	particleLifeSpanData.force = rotatedForce;
}

void ParticleSystem::addParticleToList(ParticleLifespanData& particleLifeSpanData, ParticleVertex& particleVertex, TypedResourceID<Texture> texture)
{
	std::vector<TypedResourceID<Texture>>::iterator iter = std::find(std::begin(usedTextures), std::end(usedTextures), texture);
	int textureIndex;
	if (iter == std::end(usedTextures)) {
		usedTextures.push_back(texture);
		textureIndex = static_cast<int>(usedTextures.size() - 1);
	}
	else
		textureIndex = static_cast<int>(iter - std::begin(usedTextures));

	int offset = (textureIndex * MAX_PARTICLESPERTEXTURE + counterInEachTextures[textureIndex]);
	glNamedBufferSubData(particleVerticesBO.id(), offset * sizeof(ParticleVertex), sizeof(ParticleVertex), &particleVertex);
	glNamedBufferSubData(particlesSSBO.id(), offset * sizeof(ParticleLifespanData) + alignof(ParticleLifespanData), sizeof(ParticleLifespanData), &particleLifeSpanData);
	counterInEachTextures[textureIndex]++;

	if (counterInEachTextures[textureIndex] >= MAX_PARTICLESPERTEXTURE) {
		counterInEachTextures[textureIndex] = 0;
	}
}

std::vector<PointLightData> ParticleSystem::getParticleLights(int count)
{
	particleFindLightsComputeShader.use();
	particleFindLightsComputeShader.setUInt("maxSearchableLight", count);
	bool b_Exceeded{};
	int lightParticleCount{};
	glNamedBufferSubData(particleLightsSSBO.id(), 0, sizeof(int), &lightParticleCount);
	glNamedBufferSubData(particleLightsSSBO.id(), sizeof(int), sizeof(bool), &b_Exceeded);
	int numTextures{ static_cast<int>(usedTextures.size()) };
	if (numTextures != 0) {
		glDispatchCompute((numTextures * MAX_PARTICLESPERTEXTURE) / LOCALWORKGROUPSIZE + 1, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}
	glGetNamedBufferSubData(particleLightsSSBO.id(), 0, sizeof(int), &lightParticleCount);
	glGetNamedBufferSubData(particleLightsSSBO.id(), sizeof(int), sizeof(bool), &b_Exceeded);
	std::vector<PointLightData> particleLights(lightParticleCount);
	glGetNamedBufferSubData(particleLightsSSBO.id(), alignof(PointLightData), lightParticleCount * sizeof(PointLightData), particleLights.data());
	if(b_Exceeded)
		Logger::warn("Unable to add more particle lights, max number of point lights reached!");
	return particleLights;
}

BufferObject const& ParticleSystem::getParticeVerticesBO()
{
	return particleVerticesBO;
}

void ParticleSystem::spawnParticle(Transform const& transform, ParticleEmitter& emitter)
{
	ParticleLifespanData particleLifeSpanData{};
	ParticleVertex particleVertex{};
	determineParticleSpawnDetails(
		particleLifeSpanData,
		particleVertex,
		transform.position, 
		emitter, 
		emitter.particleEmissionTypeSelection.emissionShape
	);
	determineParticleColor(
		particleLifeSpanData,
		particleVertex,
		emitter,
		emitter.particleColorSelection.emissiveMultiplier,
		emitter.particleColorSelection.color,
		emitter.colorOverLifetime.endColor,
		emitter.particleColorSelection.colorOffsetMin,
		emitter.particleColorSelection.colorOffsetMax
	);
	determineParticleSize(
		particleLifeSpanData,
		particleVertex,
		emitter,
		emitter.startSize,
		emitter.sizeOverLifetime.endSize,
		emitter.minStartSizeOffset,
		emitter.maxStartSizeOffset
	);
	rotateParticle(particleLifeSpanData,particleVertex,transform);
	addParticleToList(particleLifeSpanData, particleVertex,emitter.texture);
}



/******************************************************************************
	For Scripts
******************************************************************************/
void ParticleSystem::emit(Transform const& transform, ParticleEmitter& emitter, int count)
{
	while (count--)
		spawnParticle(transform, emitter);
}