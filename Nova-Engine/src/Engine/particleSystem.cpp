#include "particleSystem.h"
#include "engine.h"
#include "RandomRange.h"
ParticleSystem::ParticleSystem(Engine& p_engine)
	:engine{p_engine}{}

void ParticleSystem::update(float dt)
{
	for (auto&& [entity, transform, emitter] : engine.ecs.registry.view<Transform, ParticleEmitter>().each()) {
		if (!emitter.looping)
			continue;
		continuousGeneration(transform,emitter,dt);
		burstGeneration(transform, emitter, dt);
		particleMovement(emitter,dt);
	}
}

void ParticleSystem::continuousGeneration(Transform const& transform, ParticleEmitter& emitter, float dt)
{
	if (emitter.particleRate <= 0)
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
	if (emitter.burstRate <= 0)
		return;
	if (emitter.particles.size() < emitter.maxParticles)
		emitter.currentBurstTime -= dt;
	while (emitter.currentBurstTime <= 0 && emitter.particles.size() < emitter.maxParticles) {
		emitter.currentBurstTime += 1.f / emitter.burstRate;
		emit(transform, emitter);
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
		return false;
	});
	// Remove once end of lifetime
	if (it != std::end(emitter.particles)) {
		emitter.particles.erase(it, std::end(emitter.particles));
	}
}

void ParticleSystem::spawnParticle(Transform const& transform, ParticleEmitter& emitter)
{
	// To Do, Make this affected by rotation
	switch (emitter.particleEmissionTypeSelection.emissionShape) {
		case ParticleEmissionTypeSelection::EmissionShape::Sphere:
		{
			glm::vec3 randomSpawnDirection = glm::vec3(RandomRange::Float(-1, 1), RandomRange::Float(-1, 1), RandomRange::Float(-1, 1));
			randomSpawnDirection = glm::normalize(randomSpawnDirection);
			emitter.particles.push_back(Particle{ transform.position + randomSpawnDirection * RandomRange::Float(0, emitter.particleEmissionTypeSelection.radiusEmitter.radius),
				randomSpawnDirection * emitter.startSpeed, emitter.startSize,emitter.lifeTime });
			break;
		}
		case ParticleEmissionTypeSelection::EmissionShape::Point:
		{
			glm::vec3 randomVelocity = glm::vec3(RandomRange::Float(-1, 1), RandomRange::Float(-1, 1), RandomRange::Float(-1, 1));
			randomVelocity = glm::normalize(randomVelocity);
			randomVelocity *= emitter.startSpeed;
			emitter.particles.push_back(Particle{ transform.position, randomVelocity ,emitter.startSize,emitter.lifeTime });
			break;
		}
		case ParticleEmissionTypeSelection::EmissionShape::Cube:
		{
			glm::vec3 min{ emitter.particleEmissionTypeSelection.cubeEmitter.min }, max{ emitter.particleEmissionTypeSelection.cubeEmitter.max };
			glm::vec3 randomSpawnPoint = transform.position + glm::vec3{ RandomRange::Float(min.x,max.x),RandomRange::Float(min.y,max.y),RandomRange::Float(min.z,max.z) };
			emitter.particles.push_back(Particle{ randomSpawnPoint, glm::normalize(randomSpawnPoint-transform.position)* emitter.startSpeed,
				emitter.startSize,emitter.lifeTime});
			break;
		}
		case ParticleEmissionTypeSelection::EmissionShape::Edge:
		{
			glm::vec3 randomSpawnPoint = transform.position;
			randomSpawnPoint -= glm::vec3{ 1,0,0 } *emitter.particleEmissionTypeSelection.radiusEmitter.radius / 2.f;
			randomSpawnPoint += glm::vec3{ 1,0,0 } *RandomRange::Float(0, emitter.particleEmissionTypeSelection.radiusEmitter.radius);
			emitter.particles.push_back(Particle{ randomSpawnPoint, glm::vec3{0,1,0} *emitter.startSpeed ,emitter.startSize,emitter.lifeTime });
			break;
		}
		case ParticleEmissionTypeSelection::EmissionShape::Circle:
		{
			glm::vec3 randomSpawnDirection = glm::vec3(RandomRange::Float(-1, 1), 0, RandomRange::Float(-1, 1));
			randomSpawnDirection = glm::normalize(randomSpawnDirection);
			emitter.particles.push_back(Particle{ transform.position + randomSpawnDirection * RandomRange::Float(0, emitter.particleEmissionTypeSelection.radiusEmitter.radius),
				randomSpawnDirection * emitter.startSpeed, emitter.startSize,emitter.lifeTime });
			break;
		}
		case ParticleEmissionTypeSelection::EmissionShape::Hemisphere:
		{
			glm::vec3 randomSpawnDirection = glm::vec3(RandomRange::Float(-1, 1), RandomRange::Float(0, 1), RandomRange::Float(-1, 1));
			randomSpawnDirection = glm::normalize(randomSpawnDirection);
			emitter.particles.push_back(Particle{ transform.position + randomSpawnDirection * RandomRange::Float(0, emitter.particleEmissionTypeSelection.radiusEmitter.radius),
				randomSpawnDirection * emitter.startSpeed, emitter.startSize,emitter.lifeTime });
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
			// Spawn Particle
			emitter.particles.push_back(Particle{ spawnPosition, velocity, emitter.startSize,emitter.lifeTime });
			break;
		}
	}
	// Change Particles position and velocity based on tranform rotation
	glm::vec3& rotatedPosition{ emitter.particles[emitter.particles.size() - 1].position };
	rotatedPosition = getRotatedParticleSpawnPoint(transform, rotatedPosition);
	glm::vec3& rotatedVelocity{ emitter.particles[emitter.particles.size() - 1].velocity };
	rotatedVelocity = getRotatedParticleVelocity(transform, rotatedVelocity);
}

glm::vec3 ParticleSystem::getRotatedParticleSpawnPoint(Transform const& transform, glm::vec3 position)
{
	glm::mat4 model = glm::identity<glm::mat4>();
	model = glm::translate(model, -transform.position);
	model = glm::mat4_cast(transform.rotation) * model;
	glm::vec4 rotatedPosFromOrigin = glm::vec4(position, 1.0);
	rotatedPosFromOrigin = model * rotatedPosFromOrigin;
	// Not same coordinate system using glm::translate(Affectted by rotation)
	return glm::vec3{ rotatedPosFromOrigin.x,rotatedPosFromOrigin.y,rotatedPosFromOrigin.z } + transform.position;
}

glm::vec3 ParticleSystem::getRotatedParticleVelocity(Transform const& transform, glm::vec3 velocity)
{
	glm::vec4 rotatedVelocity = glm::vec4(velocity, 1.0);
	rotatedVelocity = glm::mat4_cast(transform.rotation) * rotatedVelocity;
	return glm::vec3{ rotatedVelocity.x,rotatedVelocity.y,rotatedVelocity.z };
}

/******************************************************************************
	For Scripts
******************************************************************************/
void ParticleSystem::emit(Transform const& transform, ParticleEmitter& emitter)
{
	int count{ emitter.burstAmount };
	while (count-- && emitter.particles.size() < emitter.maxParticles)
		spawnParticle(transform,emitter);
}