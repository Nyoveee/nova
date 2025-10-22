#include "particleSystem.h"
#include "engine.h"
#include "RandomRange.h"
#include "Interpolation.h"


ParticleSystem::ParticleSystem(Engine& p_engine)
	:engine{p_engine}
{
	interpolationType[InterpolationType::Root] = 0.5f;
	interpolationType[InterpolationType::Linear] = 1.0f;
	interpolationType[InterpolationType::Quadractic] = 2.0f;
	interpolationType[InterpolationType::Cubic] = 3.0f;
}

void ParticleSystem::update(float dt)
{
	for (auto&& [entity, transform, emitter] : engine.ecs.registry.view<Transform, ParticleEmitter>().each()) {
		continuousGeneration(transform,emitter,dt);
		burstGeneration(transform, emitter, dt);
		particleMovement(emitter,dt);
		particleOverLifeTime(emitter);
	}
}

void ParticleSystem::continuousGeneration(Transform const& transform, ParticleEmitter& emitter, float dt)
{
	if (emitter.particleRate <= 0 || !emitter.looping)
		return;
	if (emitter.particles.size() < emitter.maxParticles)
		emitter.currentContinuousTime -= dt;
	while (emitter.currentContinuousTime <= 0 && emitter.particles.size() < emitter.maxParticles) {
		emitter.currentContinuousTime += 1.f / emitter.particleRate;
		spawnParticle(transform,emitter);
	}
}

void ParticleSystem::burstGeneration(Transform const& transform, ParticleEmitter& emitter, float dt)
{
	if (emitter.burstRate <= 0 || !emitter.looping)
		return;
	if (emitter.particles.size() < emitter.maxParticles)
		emitter.currentBurstTime -= dt;
	while (emitter.currentBurstTime <= 0 && emitter.particles.size() < emitter.maxParticles) {
		emitter.currentBurstTime += 1.f / emitter.burstRate;
		emit(transform, emitter, emitter.burstAmount);
	}
		
}

void ParticleSystem::particleMovement(ParticleEmitter& emitter, float dt)
{
	// Move the particles
	std::vector<Particle>::iterator it = std::remove_if(std::begin(emitter.particles), std::end(emitter.particles), [&emitter, dt](Particle& particle) {
		particle.currentLifeTime -= dt;
		if (particle.currentLifeTime <= 0)
			return true;
		particle.position += particle.velocity * dt;
		particle.velocity += emitter.force * dt;
		particle.rotation += emitter.angularVelocity * dt;
		return false;
	});
	// Remove once end of lifetime
	if (it != std::end(emitter.particles)) {
		emitter.particles.erase(it, std::end(emitter.particles));
	}
}

void ParticleSystem::particleOverLifeTime(ParticleEmitter& emitter)
{
	if (emitter.lifeTime <= 0)
		return;
	if (emitter.sizeOverLifetime.selected) {
		for (Particle& particle : emitter.particles) {
			float endSize{ emitter.sizeOverLifetime.endSize };
			float interpolationValue{ 1 - particle.currentLifeTime / emitter.lifeTime };
			float degree{ interpolationType[emitter.sizeOverLifetime.interpolationType] };
			particle.currentSize = Interpolation::Interpolation(particle.startSize, endSize, interpolationValue, degree);
		}
	}
	if (emitter.colorOverLifetime.selected) {
		for (Particle& particle : emitter.particles) {
			glm::vec4 endColor{ emitter.colorOverLifetime.endColor };
			float interpolationValue{ 1 - particle.currentLifeTime / emitter.lifeTime };
			float degree{ interpolationType[emitter.sizeOverLifetime.interpolationType] };
			particle.currentColor = Interpolation::Interpolation(particle.startColor, endColor, interpolationValue, degree);
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
	// To Do, Make this affected by rotation
	Particle newParticle{};
	newParticle.startSize = newParticle.currentSize = emitter.startSize;
	newParticle.currentLifeTime = emitter.lifeTime;
	newParticle.startColor = newParticle.currentColor = emitter.particleColorSelection.color;
	if (emitter.particleColorSelection.randomizedColor)
		newParticle.startColor = newParticle.currentColor = ColorA(RandomRange::Float(0, 1), RandomRange::Float(0, 1), RandomRange::Float(0, 1), RandomRange::Float(0, 1));
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
			float radius = radiusEmitter.radius,distance = coneEmitter.distance;
			float arc = Radian{ Degree{std::clamp(coneEmitter.arc, 0.f, 75.f)}};
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
	// Change Particles position and velocity based on tranform rotation
	newParticle.position = rotateParticleSpawnPoint(transform, newParticle.position);
	newParticle.velocity = rotateParticleVelocity(transform, newParticle.velocity);
	// Create the new particle
	emitter.particles.push_back(newParticle);
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
	while (count-- && emitter.particles.size() < emitter.maxParticles)
		spawnParticle(transform,emitter);
}