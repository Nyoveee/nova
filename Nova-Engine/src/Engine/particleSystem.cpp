#include "particleSystem.h"
#include "engine.h"
#include "RandomRange.h"
#include "Interpolation.h"
#include <algorithm>
#undef max

ParticleSystem::ParticleSystem(Engine& p_engine)
	: engine{ p_engine }
	, particles{}
	, interpolationType{}
{
	interpolationType[InterpolationType::Root] = 0.5f;
	interpolationType[InterpolationType::Linear] = 1.0f;
	interpolationType[InterpolationType::Quadractic] = 2.0f;
	interpolationType[InterpolationType::Cubic] = 3.0f;
}

void ParticleSystem::update(float dt)
{
	particleMovement(dt);
	particleOverLifeTime();
	for (auto&& [entity, transform, entityData, emitter] : engine.ecs.registry.view<Transform, EntityData, ParticleEmitter>().each()){ 
		if (!entityData.isActive || !engine.ecs.isComponentActive<ParticleEmitter>(entity))
			continue;
		continuousGeneration(transform, emitter, dt);
		burstGeneration(transform, emitter, dt);
		trailGeneration(transform, emitter);
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
			Particle newParticle{};
			newParticle.type = Particle::Type::Trail;
			newParticle.emitter = &emitter;
			newParticle.emitterTransform = &transform;
			// Size
			newParticle.sizeInterpolation = interpolationType[emitter.sizeOverLifetime.interpolationType];
			newParticle.startSize = newParticle.currentSize = emitter.trails.trailSize;
			newParticle.sizeOverLifetime = emitter.sizeOverLifetime.selected;
			newParticle.endSize = emitter.sizeOverLifetime.endSize;
			// Color
			glm::vec4 color = emitter.trails.trailColor;
			glm::vec3 colorOffsetMin = emitter.trails.trailColorOffsetMin;
			glm::vec3 colorOffsetMax = emitter.trails.trailColorOffsetMax;
			ColorA startColor = color + glm::vec4(glm::vec3(RandomRange::Vec3(colorOffsetMin, colorOffsetMax)), 0);
			startColor = glm::vec4(startColor.r(), startColor.g(), startColor.b(), 0) * emitter.trails.trailEmissiveMultiplier
				+ glm::vec4(0, 0, 0, startColor.a());
			newParticle.colorInterpolation = interpolationType[emitter.colorOverLifetime.interpolationType];
			newParticle.startColor = newParticle.currentColor = startColor;
			newParticle.colorOverLifetime = emitter.colorOverLifetime.selected;
			newParticle.endColor = emitter.colorOverLifetime.endColor;
			// Lifetime
			newParticle.currentLifeTime = newParticle.lifeTime = emitter.lifeTime;
			// Other Particle Details
			newParticle.position = startPosition + direction * (distancePerEmission * index);
			newParticle.angularVelocity = emitter.initialAngularVelocity + RandomRange::Float(emitter.minAngularVelocityOffset, emitter.maxAngularVelocityOffset);
			newParticle.force = emitter.force;
			newParticle.lightIntensity = emitter.lightIntensity;
			newParticle.lightattenuation = emitter.lightattenuation;
			// Create the Particle Trail
			particles[emitter.trails.trailTexture].push_back(newParticle);
			// Update the loop
			++index;
			maxDistance -= distancePerEmission;
		}
		emitter.prevPosition = transform.position;
	}
}

void ParticleSystem::particleMovement(float dt)
{
	auto updateParticleMovement = [&](std::vector<Particle>& particles) {
		// Move the particles
		std::vector<Particle>::iterator it = std::remove_if(std::begin(particles), std::end(particles), [dt](Particle& particle) {
			particle.currentLifeTime -= dt;
			// Set this particle to be removed
			if (particle.currentLifeTime <= 0) {
				if (particle.type == Particle::Type::Standard && particle.emitter)
					--(particle.emitter->particleCount);
				return true;
			}
			
			particle.position += particle.velocity * dt;
			glm::mat4 rotation = glm::identity<glm::mat4>();
			if(particle.emitterTransform)
				rotation = glm::mat4_cast(particle.emitterTransform->rotation) * rotation;
			glm::vec4 rotatedForce = rotation * glm::vec4{ particle.force, 0 };
			particle.velocity += glm::vec3{ rotatedForce.x, rotatedForce.y,rotatedForce.z } *dt;
			particle.rotation += particle.angularVelocity * dt;
			return false;
		});
		// Remove once end of lifetime
		if (it != std::end(particles))
			particles.erase(it, std::end(particles));
	};
	for (auto&& [textureid, textureParticles] : particles) {
		(void)textureid;
		updateParticleMovement(textureParticles);
	}
	
}

void ParticleSystem::particleOverLifeTime()
{
	for (auto&& [textureid, textureParticles] : particles) {
		(void)textureid;
		for (Particle& particle : textureParticles) {
			// Size Interpolation
			if (particle.sizeOverLifetime) {
				float interpolationValue{ 1 - particle.currentLifeTime / particle.lifeTime };
				float degree{ particle.sizeInterpolation };
				particle.currentSize = Interpolation::Interpolation(particle.startSize, particle.endSize, interpolationValue, degree);
			}
			// Color Interpolation
			if (particle.colorOverLifetime) {
				float interpolationValue{ 1 - particle.currentLifeTime / particle.lifeTime };
				float degree{ particle.colorInterpolation };
				particle.currentColor = Interpolation::Interpolation(particle.startColor, particle.endColor, interpolationValue, degree);
			}
		}
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
	Particle newParticle{};
	newParticle.type = Particle::Type::Standard;
	newParticle.emitter = &emitter;
	newParticle.emitterTransform = &transform;
	// Size
	float startSize = emitter.startSize + RandomRange::Float(emitter.minStartSizeOffset, emitter.maxStartSizeOffset);
	newParticle.sizeInterpolation = interpolationType[emitter.sizeOverLifetime.interpolationType];
	newParticle.startSize = newParticle.currentSize = startSize;
	newParticle.colorOverLifetime = emitter.colorOverLifetime.selected;
	newParticle.endSize = emitter.sizeOverLifetime.endSize;
	// Lifetime
	newParticle.currentLifeTime = newParticle.lifeTime = emitter.lifeTime;
	// Color
	newParticle.startColor = newParticle.currentColor = emitter.particleColorSelection.color;
	glm::vec4 color = emitter.particleColorSelection.color;
	glm::vec3 colorOffsetMin = emitter.particleColorSelection.colorOffsetMin;
	glm::vec3 colorOffsetMax = emitter.particleColorSelection.colorOffsetMax;
	ColorA startColor = color + glm::vec4(glm::vec3(RandomRange::Vec3(colorOffsetMin, colorOffsetMax)), 0);
	startColor = glm::vec4(startColor.r(),startColor.g(),startColor.b(),0) * emitter.particleColorSelection.emissiveMultiplier 
		+ glm::vec4(0,0,0,startColor.a());
	newParticle.colorInterpolation = interpolationType[emitter.colorOverLifetime.interpolationType];
	newParticle.startColor = newParticle.currentColor = startColor;
	newParticle.colorOverLifetime = emitter.colorOverLifetime.selected;
	newParticle.endColor = emitter.colorOverLifetime.endColor;
	// Spawn Based on the shape
	switch (emitter.particleEmissionTypeSelection.emissionShape) {
	case ParticleEmissionTypeSelection::EmissionShape::Sphere:
	{
		glm::vec3 randomSpawnDirection = glm::vec3(RandomRange::Float(-1, 1), RandomRange::Float(-1, 1), RandomRange::Float(-1, 1));
		randomSpawnDirection = glm::normalize(randomSpawnDirection);
		newParticle.position = transform.position + randomSpawnDirection * RandomRange::Float(0, emitter.particleEmissionTypeSelection.radiusEmitter.radius);
		newParticle.velocity = determineParticleVelocity(emitter, randomSpawnDirection * emitter.startSpeed);
		break;
	}
	case ParticleEmissionTypeSelection::EmissionShape::Point:
	{
		glm::vec3 randomVelocity = glm::vec3(RandomRange::Float(-1, 1), RandomRange::Float(-1, 1), RandomRange::Float(-1, 1));
		randomVelocity = glm::normalize(randomVelocity);
		randomVelocity *= emitter.startSpeed;
		newParticle.position = transform.position;
		newParticle.velocity = randomVelocity;
		break;
	}
	case ParticleEmissionTypeSelection::EmissionShape::Cube:
	{
		glm::vec3 min{ emitter.particleEmissionTypeSelection.cubeEmitter.min }, max{ emitter.particleEmissionTypeSelection.cubeEmitter.max };
		glm::vec3 randomSpawnPoint = transform.position + glm::vec3{ RandomRange::Float(min.x,max.x),RandomRange::Float(min.y,max.y),RandomRange::Float(min.z,max.z) };
		newParticle.position = randomSpawnPoint;
		newParticle.velocity = determineParticleVelocity(emitter, glm::normalize(randomSpawnPoint - transform.position) * emitter.startSpeed);
		break;
	}
	case ParticleEmissionTypeSelection::EmissionShape::Edge:
	{
		glm::vec3 randomSpawnPoint = transform.position;
		randomSpawnPoint -= glm::vec3{ 1,0,0 } *emitter.particleEmissionTypeSelection.radiusEmitter.radius / 2.f;
		randomSpawnPoint += glm::vec3{ 1,0,0 } *RandomRange::Float(0, emitter.particleEmissionTypeSelection.radiusEmitter.radius);
		newParticle.position = randomSpawnPoint;
		newParticle.velocity = determineParticleVelocity(emitter, glm::vec3{ 0,1,0 } *emitter.startSpeed);
		break;
	}
	case ParticleEmissionTypeSelection::EmissionShape::Circle:
	{
		glm::vec3 randomSpawnDirection = glm::vec3(RandomRange::Float(-1, 1), 0, RandomRange::Float(-1, 1));
		randomSpawnDirection = glm::normalize(randomSpawnDirection);
		newParticle.position = transform.position + randomSpawnDirection * RandomRange::Float(0, emitter.particleEmissionTypeSelection.radiusEmitter.radius);
		newParticle.velocity = determineParticleVelocity(emitter, randomSpawnDirection * emitter.startSpeed);
		break;
	}
	case ParticleEmissionTypeSelection::EmissionShape::Hemisphere:
	{
		glm::vec3 randomSpawnDirection = glm::vec3(RandomRange::Float(-1, 1), RandomRange::Float(0, 1), RandomRange::Float(-1, 1));
		randomSpawnDirection = glm::normalize(randomSpawnDirection);
		newParticle.position = transform.position + randomSpawnDirection * RandomRange::Float(0, emitter.particleEmissionTypeSelection.radiusEmitter.radius);
		newParticle.velocity = determineParticleVelocity(emitter, randomSpawnDirection * emitter.startSpeed);
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
		newParticle.position = spawnPosition;
		newParticle.velocity = determineParticleVelocity(emitter, velocity);
		break;
	}
	}
	// Other Particle Details
	newParticle.position = rotateParticleSpawnPoint(transform, newParticle.position);
	newParticle.velocity = rotateParticleVelocity(transform, newParticle.velocity);
	newParticle.angularVelocity = emitter.initialAngularVelocity + RandomRange::Float(emitter.minAngularVelocityOffset, emitter.maxAngularVelocityOffset);
	newParticle.force = emitter.force;
	newParticle.lightIntensity = emitter.lightIntensity;
	newParticle.lightattenuation = emitter.lightattenuation;
	// Create the new particle
	particles[emitter.texture].push_back(newParticle);
	++emitter.particleCount;
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