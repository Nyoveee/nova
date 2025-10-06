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
		// Updating of particles
		std::vector<Particle>::iterator it = std::remove_if(std::begin(emitter.particles), std::end(emitter.particles), [&emitter, dt](Particle& particle) {
			particle.currentLifeTime -= dt;
			if (particle.currentLifeTime <= 0)
				return true;
			particle.position += particle.velocity * dt;
			particle.velocity += emitter.force * dt;
			return false;
			});
		if (it != std::end(emitter.particles)) {
			emitter.particles.erase(it,std::end(emitter.particles));
		}
		// Creation of particles
		if (emitter.particleRate <= 0)
			continue;
		if (emitter.particles.size() < emitter.maxParticles)
			emitter.currentTime -= dt;
		while (emitter.currentTime <= 0 && emitter.particles.size() < emitter.maxParticles) {
			emitter.currentTime += 1.f / emitter.particleRate;
			glm::vec3 randomDirection = glm::vec3(RandomRange::Float(-1, 1), RandomRange::Float(-1, 1), RandomRange::Float(-1, 1));
			randomDirection = glm::normalize(randomDirection);
			Particle newParticle = { transform.position, randomDirection * emitter.startSpeed ,emitter.startSize,emitter.lifeTime };
			emitter.particles.push_back(newParticle);
		}
	}
}
