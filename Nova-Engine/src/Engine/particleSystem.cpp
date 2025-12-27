#include "particleSystem.h"
#include "engine.h"
#include "RandomRange.h"
#include "Interpolation.h"

#include <algorithm>

#undef max
// Hopefully should be enough
constexpr int MAX_DELETIONPERCOMPUTE = 100000;
constexpr int MAX_PARTICLESPERTEXTURE = 100000;

ParticleSystem::ParticleSystem(Engine& p_engine)
	: engine{ p_engine }
	, particles{}
	, interpolationType{}
	, deletionList{ std::vector<unsigned int>(MAX_DELETIONPERCOMPUTE) }
	, numberOfDeletions{-1}
	, particleUpdateComputeShader("System/Shader/particleupdate.compute")
	, particleSSBO{MAX_PARTICLESPERTEXTURE * sizeof(ParticleLifespanData) + alignof(ParticleLifespanData)}
	, particleDeletionListSSBO{ MAX_DELETIONPERCOMPUTE * sizeof(unsigned int) + sizeof(unsigned int)}
{
	interpolationType[InterpolationType::Root] = 0.5f;
	interpolationType[InterpolationType::Linear] = 1.0f;
	interpolationType[InterpolationType::Quadractic] = 2.0f;
	interpolationType[InterpolationType::Cubic] = 3.0f;
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
	// Bind the respective ssbo to index
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particleSSBO.id());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, particleDeletionListSSBO.id());
	// Updating of Particles
	auto updateParticles = [&](TypedResourceID<Texture> const& textureID) {
		std::vector<ParticleLifespanData>& textureParticlesLifeSpanDatas{ particleLifeSpanDatas[textureID] };
		std::vector<Particle>& textureParticles{ particles[textureID] };
		// Send the particles to compute shader to update
		unsigned int particleAmount{ static_cast<unsigned int>(textureParticlesLifeSpanDatas.size()) };
		if (particleAmount == 0)
			return;
		glNamedBufferSubData(particleSSBO.id(), 0, sizeof(unsigned int), &particleAmount);
		glNamedBufferSubData(particleSSBO.id(), alignof(ParticleLifespanData), particleAmount * sizeof(ParticleLifespanData), textureParticlesLifeSpanDatas.data());
		glNamedBufferSubData(particleDeletionListSSBO.id(), 0, sizeof(unsigned int), &numberOfDeletions);
		glNamedBufferSubData(particleDeletionListSSBO.id(), sizeof(unsigned int), MAX_DELETIONPERCOMPUTE * sizeof(unsigned int), deletionList.data());
		// Update the particle over life span
		glDispatchCompute(particleAmount, 1, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);
		// Get back the data
		glGetNamedBufferSubData(particleSSBO.id(), 0, sizeof(unsigned int), &particleAmount);
		glGetNamedBufferSubData(particleSSBO.id(), alignof(ParticleLifespanData), particleAmount * sizeof(ParticleLifespanData), textureParticlesLifeSpanDatas.data());
		glGetNamedBufferSubData(particleDeletionListSSBO.id(), 0, sizeof(unsigned int), &numberOfDeletions);
		glGetNamedBufferSubData(particleDeletionListSSBO.id(), sizeof(unsigned int), MAX_DELETIONPERCOMPUTE * sizeof(unsigned int), deletionList.data());
		// Delete the List of Indexes
		std::sort(std::begin(deletionList), std::end(deletionList), std::greater<int>());
		for (int i{}; i < std::min(0,numberOfDeletions);++i) {
			unsigned int deletionIndex = deletionList[i];
			Particle& particle = textureParticles[deletionIndex];
			if (particle.type == Particle::Type::Standard && particle.emitter)
				--(particle.emitter->particleCount);
			textureParticles.erase(std::begin(textureParticles) + deletionIndex);
			textureParticlesLifeSpanDatas.erase(std::begin(textureParticlesLifeSpanDatas) + deletionIndex);
		}
		numberOfDeletions = -1;
	};
	for (auto&& [textureid, mapValue] : engine.particleSystem.particleLifeSpanDatas) {
		(void)mapValue;
		updateParticles(textureid);
	}
	
}

void ParticleSystem::continuousGeneration(Transform const& transform, ParticleEmitter& emitter, float dt)
{
	if (emitter.particleRate <= 0 || !emitter.looping)
		return;
	if (emitter.particleCount < emitter.maxParticles)
		emitter.currentContinuousTime -= dt;
	while (emitter.currentContinuousTime <= 0 && emitter.particleCount < emitter.maxParticles) {
		emitter.currentContinuousTime += 1.f / emitter.particleRate;
		spawnParticle(transform, emitter);
	}
}

void ParticleSystem::burstGeneration(Transform const& transform, ParticleEmitter& emitter, float dt)
{
	if (emitter.burstRate <= 0 || !emitter.looping)
		return;
	if (emitter.particleCount < emitter.maxParticles)
		emitter.currentBurstTime -= dt;
	while (emitter.currentBurstTime <= 0 && emitter.particleCount < emitter.maxParticles) {
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
			ParticleLifespanData particleLifespanData{};
			// Size
			particleLifespanData.sizeInterpolation = interpolationType[emitter.sizeOverLifetime.interpolationType];
			particleLifespanData.startSize = particleLifespanData.currentSize = emitter.trails.trailSize;
			particleLifespanData.sizeOverLifetime = emitter.sizeOverLifetime.selected;
			particleLifespanData.endSize = emitter.sizeOverLifetime.endSize;
			// Color
			glm::vec4 color = emitter.trails.trailColor;
			glm::vec3 colorOffsetMin = emitter.trails.trailColorOffsetMin;
			glm::vec3 colorOffsetMax = emitter.trails.trailColorOffsetMax;
			ColorA startColor = color + glm::vec4(glm::vec3(RandomRange::Vec3(colorOffsetMin, colorOffsetMax)), 0);
			startColor = glm::vec4(startColor.r(), startColor.g(), startColor.b(), 0) * emitter.trails.trailEmissiveMultiplier
				+ glm::vec4(0, 0, 0, startColor.a());
			particleLifespanData.colorInterpolation = interpolationType[emitter.colorOverLifetime.interpolationType];
			particleLifespanData.startColor = particleLifespanData.currentColor = startColor;
			particleLifespanData.colorOverLifetime = emitter.colorOverLifetime.selected;
			particleLifespanData.endColor = emitter.colorOverLifetime.endColor;
			// Lifetime
			particleLifespanData.currentLifeTime = particleLifespanData.lifeTime = emitter.lifeTime;
			// Other Particle Details
			particleLifespanData.position = startPosition + direction * (distancePerEmission * index);
			particleLifespanData.angularVelocity = emitter.initialAngularVelocity + RandomRange::Float(emitter.minAngularVelocityOffset, emitter.maxAngularVelocityOffset);
			glm::mat4 rotation = glm::mat4_cast(transform.rotation) * glm::identity<glm::mat4>();
			glm::vec4 rotatedForce = rotation * glm::vec4{ emitter.force, 0 };
			particleLifespanData.force = rotatedForce;
			particleLifespanData.lightIntensity = emitter.lightIntensity;
			particleLifespanData.lightattenuation = emitter.lightattenuation;
			// Create the Particle Trail
			particleLifeSpanDatas[emitter.trails.trailTexture].push_back(particleLifespanData);
			particles[emitter.trails.trailTexture].push_back(Particle{ &emitter, Particle::Type::Trail });
			// Update the loop
			++index;
			maxDistance -= distancePerEmission;
		}
		emitter.prevPosition = transform.position;
	}
}

glm::vec3 ParticleSystem::determineParticleVelocity(ParticleEmitter& emitter, glm::vec3 nonRandomizedDirection)
{
	if (!emitter.randomizedDirection)
		return nonRandomizedDirection;
	return glm::vec3(RandomRange::Float(-1, 1), RandomRange::Float(-1, 1), RandomRange::Float(-1, 1)) * emitter.startSpeed;
}

void ParticleSystem::spawnParticle(Transform const& transform, ParticleEmitter& emitter)
{
	ParticleLifespanData particleLifespanData{};
	// Size
	float startSize = emitter.startSize + RandomRange::Float(emitter.minStartSizeOffset, emitter.maxStartSizeOffset);
	particleLifespanData.sizeInterpolation = interpolationType[emitter.sizeOverLifetime.interpolationType];
	particleLifespanData.startSize = particleLifespanData.currentSize = startSize;
	particleLifespanData.colorOverLifetime = emitter.colorOverLifetime.selected;
	particleLifespanData.endSize = emitter.sizeOverLifetime.endSize;
	// Lifetime
	particleLifespanData.currentLifeTime = particleLifespanData.lifeTime = emitter.lifeTime;
	// Color
	particleLifespanData.startColor = particleLifespanData.currentColor = emitter.particleColorSelection.color;
	glm::vec4 color = emitter.particleColorSelection.color;
	glm::vec3 colorOffsetMin = emitter.particleColorSelection.colorOffsetMin;
	glm::vec3 colorOffsetMax = emitter.particleColorSelection.colorOffsetMax;
	ColorA startColor = color + glm::vec4(glm::vec3(RandomRange::Vec3(colorOffsetMin, colorOffsetMax)), 0);
	startColor = glm::vec4(startColor.r(),startColor.g(),startColor.b(),0) * emitter.particleColorSelection.emissiveMultiplier 
		+ glm::vec4(0,0,0,startColor.a());
	particleLifespanData.colorInterpolation = interpolationType[emitter.colorOverLifetime.interpolationType];
	particleLifespanData.startColor = particleLifespanData.currentColor = startColor;
	particleLifespanData.colorOverLifetime = emitter.colorOverLifetime.selected;
	particleLifespanData.endColor = emitter.colorOverLifetime.endColor;
	// Spawn Based on the shape
	switch (emitter.particleEmissionTypeSelection.emissionShape) {
	case ParticleEmissionTypeSelection::EmissionShape::Sphere:
	{
		glm::vec3 randomSpawnDirection = glm::vec3(RandomRange::Float(-1, 1), RandomRange::Float(-1, 1), RandomRange::Float(-1, 1));
		randomSpawnDirection = glm::normalize(randomSpawnDirection);
		particleLifespanData.position = transform.position + randomSpawnDirection * RandomRange::Float(0, emitter.particleEmissionTypeSelection.radiusEmitter.radius);
		particleLifespanData.velocity = determineParticleVelocity(emitter, randomSpawnDirection * emitter.startSpeed);
		break;
	}
	case ParticleEmissionTypeSelection::EmissionShape::Point:
	{
		glm::vec3 randomVelocity = glm::vec3(RandomRange::Float(-1, 1), RandomRange::Float(-1, 1), RandomRange::Float(-1, 1));
		randomVelocity = glm::normalize(randomVelocity);
		randomVelocity *= emitter.startSpeed;
		particleLifespanData.position = transform.position;
		particleLifespanData.velocity = randomVelocity;
		break;
	}
	case ParticleEmissionTypeSelection::EmissionShape::Cube:
	{
		glm::vec3 min{ emitter.particleEmissionTypeSelection.cubeEmitter.min }, max{ emitter.particleEmissionTypeSelection.cubeEmitter.max };
		glm::vec3 randomSpawnPoint = transform.position + glm::vec3{ RandomRange::Float(min.x,max.x),RandomRange::Float(min.y,max.y),RandomRange::Float(min.z,max.z) };
		particleLifespanData.position = randomSpawnPoint;
		particleLifespanData.velocity = determineParticleVelocity(emitter, glm::normalize(randomSpawnPoint - transform.position) * emitter.startSpeed);
		break;
	}
	case ParticleEmissionTypeSelection::EmissionShape::Edge:
	{
		glm::vec3 randomSpawnPoint = transform.position;
		randomSpawnPoint -= glm::vec3{ 1,0,0 } *emitter.particleEmissionTypeSelection.radiusEmitter.radius / 2.f;
		randomSpawnPoint += glm::vec3{ 1,0,0 } *RandomRange::Float(0, emitter.particleEmissionTypeSelection.radiusEmitter.radius);
		particleLifespanData.position = randomSpawnPoint;
		particleLifespanData.velocity = determineParticleVelocity(emitter, glm::vec3{ 0,1,0 } *emitter.startSpeed);
		break;
	}
	case ParticleEmissionTypeSelection::EmissionShape::Circle:
	{
		glm::vec3 randomSpawnDirection = glm::vec3(RandomRange::Float(-1, 1), 0, RandomRange::Float(-1, 1));
		randomSpawnDirection = glm::normalize(randomSpawnDirection);
		particleLifespanData.position = transform.position + randomSpawnDirection * RandomRange::Float(0, emitter.particleEmissionTypeSelection.radiusEmitter.radius);
		particleLifespanData.velocity = determineParticleVelocity(emitter, randomSpawnDirection * emitter.startSpeed);
		break;
	}
	case ParticleEmissionTypeSelection::EmissionShape::Hemisphere:
	{
		glm::vec3 randomSpawnDirection = glm::vec3(RandomRange::Float(-1, 1), RandomRange::Float(0, 1), RandomRange::Float(-1, 1));
		randomSpawnDirection = glm::normalize(randomSpawnDirection);
		particleLifespanData.position = transform.position + randomSpawnDirection * RandomRange::Float(0, emitter.particleEmissionTypeSelection.radiusEmitter.radius);
		particleLifespanData.velocity = determineParticleVelocity(emitter, randomSpawnDirection * emitter.startSpeed);
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
		glm::vec3 spawnPosition = transform.position + randomSpawnDirection * spawnRadius;
		glm::vec3 targetPosition = transform.position + glm::vec3{ 0,distance,0 } + randomSpawnDirection * spawnRadius / radius * outerRadius;
		glm::vec3 velocity = glm::normalize(targetPosition - spawnPosition) * emitter.startSpeed;
		// Set the new Particle details
		particleLifespanData.position = spawnPosition;
		particleLifespanData.velocity = determineParticleVelocity(emitter, velocity);
		break;
	}
	}
	// Other Particle Details
	particleLifespanData.position = rotateParticleSpawnPoint(transform, particleLifespanData.position);
	particleLifespanData.velocity = rotateParticleVelocity(transform, particleLifespanData.velocity);
	particleLifespanData.angularVelocity = emitter.initialAngularVelocity + RandomRange::Float(emitter.minAngularVelocityOffset, emitter.maxAngularVelocityOffset);
	glm::mat4 rotation = glm::mat4_cast(transform.rotation) * glm::identity<glm::mat4>();
	glm::vec4 rotatedForce = rotation * glm::vec4{ emitter.force, 0 };
	particleLifespanData.force = rotatedForce;
	particleLifespanData.lightIntensity = emitter.lightIntensity;
	particleLifespanData.lightattenuation = emitter.lightattenuation;
	// Create the new particle
	particleLifeSpanDatas[emitter.texture].push_back(particleLifespanData);
	particles[emitter.texture].push_back(Particle{ &emitter, Particle::Type::Standard });
	++(emitter.particleCount);
}


glm::vec3 ParticleSystem::rotateParticleSpawnPoint(Transform const& transform, glm::vec3 position)
{
	glm::mat4 model = glm::identity<glm::mat4>();
	model = glm::translate(model, -transform.position);
	model = glm::mat4_cast(transform.rotation) * model;
	glm::vec4 rotatedPosFromOrigin = glm::vec4(position, 1.0);
	rotatedPosFromOrigin = model * rotatedPosFromOrigin;
	// Not same coordinate system using glm::translate(Affectted by rotation)
	return glm::vec3{ rotatedPosFromOrigin.x,rotatedPosFromOrigin.y,rotatedPosFromOrigin.z } + transform.position;
}

glm::vec3 ParticleSystem::rotateParticleVelocity(Transform const& transform, glm::vec3 velocity)
{
	glm::vec4 rotatedVelocity = glm::vec4(velocity, 1.0);
	rotatedVelocity = glm::mat4_cast(transform.rotation) * rotatedVelocity;
	return glm::vec3{ rotatedVelocity.x,rotatedVelocity.y,rotatedVelocity.z };
}

/******************************************************************************
	For Scripts
******************************************************************************/
void ParticleSystem::emit(Transform const& transform, ParticleEmitter& emitter, int count)
{
	while (count-- && emitter.particleCount < emitter.maxParticles)
		spawnParticle(transform, emitter);
}