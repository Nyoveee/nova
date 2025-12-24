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
	, maxdeletionIndex{-1}
	, particleUpdateComputeShader("System/Shader/particleupdate.compute")
	, particleSSBO{MAX_PARTICLESPERTEXTURE * sizeof(ParticleLifespanData) + alignof(ParticleLifespanData)}
	, particleDeletionListSSBO{ MAX_DELETIONPERCOMPUTE * sizeof(unsigned int) + sizeof(unsigned int)}
{
	// Bind the respective ssbo to index
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, particleSSBO.id());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, particleDeletionListSSBO.id());
	// Interpolation
	interpolationType[InterpolationType::Root] = 0.5f;
	interpolationType[InterpolationType::Linear] = 1.0f;
	interpolationType[InterpolationType::Quadractic] = 2.0f;
	interpolationType[InterpolationType::Cubic] = 3.0f;
}
void ParticleSystem::reset() {
	particles.clear();
	particleLifeSpanDatas.clear();
	maxdeletionIndex = -1;
	deletionList.clear();
	deletionList.resize(MAX_DELETIONPERCOMPUTE);
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
		glNamedBufferSubData(particleDeletionListSSBO.id(), 0, sizeof(int), &maxdeletionIndex);
		glNamedBufferSubData(particleDeletionListSSBO.id(), sizeof(unsigned int), MAX_DELETIONPERCOMPUTE * sizeof(unsigned int), deletionList.data());
		// Update the particle over life span
		glDispatchCompute(particleAmount, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		// Get back the data
		glGetNamedBufferSubData(particleSSBO.id(), 0, sizeof(unsigned int), &particleAmount);
		glGetNamedBufferSubData(particleSSBO.id(), alignof(ParticleLifespanData), particleAmount * sizeof(ParticleLifespanData), textureParticlesLifeSpanDatas.data());
		glGetNamedBufferSubData(particleDeletionListSSBO.id(), 0, sizeof(int), &maxdeletionIndex);
		glGetNamedBufferSubData(particleDeletionListSSBO.id(), sizeof(unsigned int), MAX_DELETIONPERCOMPUTE * sizeof(unsigned int), deletionList.data());
		// Delete the List of Indexes
		std::sort(std::begin(deletionList), std::begin(deletionList) + maxdeletionIndex + 1, std::greater<int>());
		for (int i{}; i < maxdeletionIndex + 1; ++i) {
			unsigned int deletionIndex = deletionList[i];
			Particle& particle = textureParticles[deletionIndex];
			if (particle.type == Particle::Type::Standard && particle.emitter)
				--(particle.emitter->particleCount);
			textureParticles.erase(std::begin(textureParticles) + deletionIndex);
			textureParticlesLifeSpanDatas.erase(std::begin(textureParticlesLifeSpanDatas) + deletionIndex);
			deletionList[i] = 0;
		}
		maxdeletionIndex = -1;
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
			ParticleLifespanData particleLifeSpanData{};
			determineParticleSpawnDetails(
				startPosition + direction * (distancePerEmission * index), 
				emitter, 
				particleLifeSpanData, 
				ParticleEmissionTypeSelection::EmissionShape::Point
			);
			determineParticleColor(
				particleLifeSpanData,
				emitter,
				emitter.trails.trailEmissiveMultiplier,
				emitter.trails.trailColor,
				emitter.colorOverLifetime.endColor,
				emitter.trails.trailColorOffsetMin,
				emitter.trails.trailColorOffsetMax
			);
			determineParticleSize(
				particleLifeSpanData,
				emitter,
				emitter.trails.trailSize,
				emitter.sizeOverLifetime.endSize,
				0,
				0
			);
			// Ignore the set velocity
			particleLifeSpanData.velocity = glm::vec3{ 0,0,0 };
			// Create the Particle Trail
			particleLifeSpanDatas[emitter.trails.trailTexture].push_back(particleLifeSpanData);
			particles[emitter.trails.trailTexture].push_back(Particle{ &emitter, Particle::Type::Trail });
			// Update the loop
			++index;
			maxDistance -= distancePerEmission;
		}
		emitter.prevPosition = transform.position;
	}
}

void ParticleSystem::determineParticleSpawnDetails(
	glm::vec3 position,
	ParticleEmitter& emitter, 
	ParticleLifespanData& particleLifeSpanData, 
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
			particleLifeSpanData.position = position + randomSpawnDirection * RandomRange::Float(0, emitter.particleEmissionTypeSelection.radiusEmitter.radius);
			particleLifeSpanData.velocity = determineParticleVelocity(emitter, randomSpawnDirection * emitter.startSpeed);
			break;
		}
		case ParticleEmissionTypeSelection::EmissionShape::Point:
		{
			glm::vec3 randomVelocity = glm::vec3(RandomRange::Float(-1, 1), RandomRange::Float(-1, 1), RandomRange::Float(-1, 1));
			randomVelocity = glm::normalize(randomVelocity);
			randomVelocity *= emitter.startSpeed;
			particleLifeSpanData.position = position;
			particleLifeSpanData.velocity = randomVelocity;
			break;
		}
		case ParticleEmissionTypeSelection::EmissionShape::Cube:
		{
			glm::vec3 min{ emitter.particleEmissionTypeSelection.cubeEmitter.min }, max{ emitter.particleEmissionTypeSelection.cubeEmitter.max };
			glm::vec3 randomSpawnPoint = position + glm::vec3{ RandomRange::Float(min.x,max.x),RandomRange::Float(min.y,max.y),RandomRange::Float(min.z,max.z) };
			particleLifeSpanData.position = randomSpawnPoint;
			particleLifeSpanData.velocity = determineParticleVelocity(emitter, glm::normalize(randomSpawnPoint - position) * emitter.startSpeed);
			break;
		}
		case ParticleEmissionTypeSelection::EmissionShape::Edge:
		{
			glm::vec3 randomSpawnPoint = position;
			randomSpawnPoint -= glm::vec3{ 1,0,0 } *emitter.particleEmissionTypeSelection.radiusEmitter.radius / 2.f;
			randomSpawnPoint += glm::vec3{ 1,0,0 } *RandomRange::Float(0, emitter.particleEmissionTypeSelection.radiusEmitter.radius);
			particleLifeSpanData.position = randomSpawnPoint;
			particleLifeSpanData.velocity = determineParticleVelocity(emitter, glm::vec3{ 0,1,0 } *emitter.startSpeed);
			break;
		}
		case ParticleEmissionTypeSelection::EmissionShape::Circle:
		{
			glm::vec3 randomSpawnDirection = glm::vec3(RandomRange::Float(-1, 1), 0, RandomRange::Float(-1, 1));
			randomSpawnDirection = glm::normalize(randomSpawnDirection);
			particleLifeSpanData.position = position + randomSpawnDirection * RandomRange::Float(0, emitter.particleEmissionTypeSelection.radiusEmitter.radius);
			particleLifeSpanData.velocity = determineParticleVelocity(emitter, randomSpawnDirection * emitter.startSpeed);
			break;
		}
		case ParticleEmissionTypeSelection::EmissionShape::Hemisphere:
		{
			glm::vec3 randomSpawnDirection = glm::vec3(RandomRange::Float(-1, 1), RandomRange::Float(0, 1), RandomRange::Float(-1, 1));
			randomSpawnDirection = glm::normalize(randomSpawnDirection);
			particleLifeSpanData.position = position + randomSpawnDirection * RandomRange::Float(0, emitter.particleEmissionTypeSelection.radiusEmitter.radius);
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
			particleLifeSpanData.position = spawnPosition;
			particleLifeSpanData.velocity = determineParticleVelocity(emitter, velocity);
			break;
		}
	}
	// Other Particle Details
	particleLifeSpanData.force = emitter.force;
	particleLifeSpanData.angularVelocity = emitter.initialAngularVelocity + RandomRange::Float(emitter.minAngularVelocityOffset, emitter.maxAngularVelocityOffset);
	particleLifeSpanData.lightIntensity = emitter.lightIntensity;
	particleLifeSpanData.lightattenuation = emitter.lightattenuation;
}

void ParticleSystem::determineParticleColor(ParticleLifespanData& particleLifeSpanData,
	ParticleEmitter& emitter,
	float emissiveMultiplier,
	ColorA startcolor, ColorA endColor,
	glm::vec3 colorOffsetMin, glm::vec3 colorOffsetMax)
{
	// Color
	particleLifeSpanData.startColor = particleLifeSpanData.currentColor = startcolor;
	glm::vec4 color = emitter.particleColorSelection.color;
	ColorA startColor = color + glm::vec4(glm::vec3(RandomRange::Vec3(colorOffsetMin, colorOffsetMax)), 0);
	startColor = glm::vec4(startColor.r(), startColor.g(), startColor.b(), 0) * emissiveMultiplier + glm::vec4(0, 0, 0, startColor.a());
	particleLifeSpanData.colorInterpolation = interpolationType[emitter.colorOverLifetime.interpolationType];
	particleLifeSpanData.startColor = particleLifeSpanData.currentColor = startColor;
	particleLifeSpanData.colorOverLifetime = emitter.colorOverLifetime.selected;
	particleLifeSpanData.endColor = endColor;
}

void ParticleSystem::determineParticleSize(ParticleLifespanData& particleLifeSpanData, 
	ParticleEmitter& emitter,
	float startSize, float endSize,
	float minStartSizeOffset, float maxStartSizeOffset)
{
	particleLifeSpanData.sizeOverLifetime = emitter.sizeOverLifetime.selected;
	particleLifeSpanData.sizeInterpolation = interpolationType[emitter.sizeOverLifetime.interpolationType];
	particleLifeSpanData.startSize = particleLifeSpanData.currentSize = startSize + RandomRange::Float(minStartSizeOffset, maxStartSizeOffset);
	particleLifeSpanData.endSize = endSize;
}

void ParticleSystem::rotateParticle(Transform const& transform, ParticleLifespanData& particleLifeSpanData)
{
	// Position
	glm::mat4 model = glm::identity<glm::mat4>();
	model = glm::translate(model, -transform.position);
	model = glm::mat4_cast(transform.rotation) * model;
	glm::vec4 rotatedPosFromOrigin = glm::vec4(particleLifeSpanData.position, 1.0);
	rotatedPosFromOrigin = model * rotatedPosFromOrigin;
	particleLifeSpanData.position = glm::vec3{ rotatedPosFromOrigin.x,rotatedPosFromOrigin.y,rotatedPosFromOrigin.z } + transform.position;
	// Velocity
	glm::vec4 rotatedVelocity = glm::vec4(particleLifeSpanData.velocity, 1.0);
	rotatedVelocity = glm::mat4_cast(transform.rotation) * rotatedVelocity;
	particleLifeSpanData.velocity = glm::vec3{ rotatedVelocity.x,rotatedVelocity.y,rotatedVelocity.z };
	// Force
	glm::mat4 rotation = glm::mat4_cast(transform.rotation) * glm::identity<glm::mat4>();
	glm::vec4 rotatedForce = rotation * glm::vec4{ particleLifeSpanData.force, 0 };
	particleLifeSpanData.force = rotatedForce;
}

void ParticleSystem::spawnParticle(Transform const& transform, ParticleEmitter& emitter)
{
	ParticleLifespanData particleLifeSpanData{};
	determineParticleSpawnDetails(
		transform.position, 
		emitter, 
		particleLifeSpanData,
		emitter.particleEmissionTypeSelection.emissionShape
	);
	determineParticleColor(
		particleLifeSpanData,
		emitter,
		emitter.particleColorSelection.emissiveMultiplier,
		emitter.particleColorSelection.color,
		emitter.colorOverLifetime.endColor,
		emitter.particleColorSelection.colorOffsetMin,
		emitter.particleColorSelection.colorOffsetMax
	);
	determineParticleSize(
		particleLifeSpanData,
		emitter,
		emitter.startSize,
		emitter.sizeOverLifetime.endSize,
		emitter.minStartSizeOffset,
		emitter.maxStartSizeOffset
	);
	rotateParticle(transform, particleLifeSpanData);
	// Create the new particle
	particleLifeSpanDatas[emitter.texture].push_back(particleLifeSpanData);
	particles[emitter.texture].push_back(Particle{ &emitter, Particle::Type::Standard });
	++(emitter.particleCount);
}



/******************************************************************************
	For Scripts
******************************************************************************/
void ParticleSystem::emit(Transform const& transform, ParticleEmitter& emitter, int count)
{
	while (count-- && emitter.particleCount < emitter.maxParticles)
		spawnParticle(transform, emitter);
}