#pragma once
#include "export.h"
#include "component.h"
#include <unordered_map>
class Engine;

class ParticleSystem
{
public:
	ENGINE_DLL_API ParticleSystem(Engine& p_engine);
	ENGINE_DLL_API ParticleSystem(ParticleSystem const& other) = delete;
	ENGINE_DLL_API ParticleSystem(ParticleSystem&& other) = delete;
	ENGINE_DLL_API ParticleSystem& operator=(ParticleSystem const& other) = delete;
	ENGINE_DLL_API ParticleSystem& operator=(ParticleSystem&& other) = delete;
public:
	ENGINE_DLL_API void emit(Transform const& transform, ParticleEmitter& emitter, int count);
public:
	void update(float dt);

private:
	// Spawning
	void continuousGeneration(Transform const& transform, ParticleEmitter& emitter, float dt);
	void burstGeneration(Transform const& transform, ParticleEmitter& emitter, float dt);
	void spawnParticle(Transform const& transform, ParticleEmitter& emitter);
	// Update
	void particleMovement(ParticleEmitter& emitter, float dt);
	void particleOverLifeTime(ParticleEmitter& emitter);
	// Particle Info
	glm::vec3 determineParticleVelocity(ParticleEmitter& emitter, glm::vec3 nonRandomizedDirection);
	// Rotation
	glm::vec3 rotateParticleSpawnPoint(Transform const& transform, glm::vec3 position);
	glm::vec3 rotateParticleVelocity(Transform const& transform, glm::vec3 velocity);
private:
	std::unordered_map<InterpolationType, float> interpolationType;
	Engine& engine;
};

