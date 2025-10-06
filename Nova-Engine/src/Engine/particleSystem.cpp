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
		glm::vec3 randomDirection = glm::vec3(RandomRange::Float(-1, 1), RandomRange::Float(-1, 1), RandomRange::Float(-1, 1));
		randomDirection = glm::normalize(randomDirection);
		Particle newParticle = { transform.position, randomDirection * emitter.startSpeed ,emitter.startSize,emitter.lifeTime };
		emitter.particles.push_back(newParticle);
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

/******************************************************************************
	For Scripts
******************************************************************************/
void ParticleSystem::emit(Transform const& transform, ParticleEmitter& emitter)
{
	int count{ emitter.burstAmount };
	while (count-- && emitter.particles.size() < emitter.maxParticles) {
		glm::vec3 randomDirection = glm::vec3(RandomRange::Float(-1, 1), RandomRange::Float(-1, 1), RandomRange::Float(-1, 1));
		randomDirection = glm::normalize(randomDirection);
		Particle newParticle = { transform.position, randomDirection * emitter.startSpeed ,emitter.startSize,emitter.lifeTime };
		emitter.particles.push_back(newParticle);
	}
}