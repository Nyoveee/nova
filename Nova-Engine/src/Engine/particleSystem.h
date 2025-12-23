#pragma once
#include <glad/glad.h>
#include <map>

#include "export.h"
#include "component.h"
#include "computeShader.h"
#include "Graphics/bufferObject.h"

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
public:
	std::map<TypedResourceID<Texture>, std::vector<Particle>> particles;
	std::map<TypedResourceID<Texture>, std::vector<ParticleLifespanData>> particleLifeSpanDatas;
private:
	// Spawning
	void continuousGeneration(Transform const& transform, ParticleEmitter& emitter, float dt);
	void burstGeneration(Transform const& transform, ParticleEmitter& emitter, float dt);
	void trailGeneration(Transform& transform, ParticleEmitter& emitter);
	void spawnParticle(Transform const& transform, ParticleEmitter& emitter);
	// Particle Info
	glm::vec3 determineParticleVelocity(ParticleEmitter& emitter, glm::vec3 nonRandomizedDirection);
	// Rotation
	glm::vec3 rotateParticleSpawnPoint(Transform const& transform, glm::vec3 position);
	glm::vec3 rotateParticleVelocity(Transform const& transform, glm::vec3 velocity);
private:
	ComputeShader particleUpdateComputeShader;
	BufferObject particleSSBO;
	BufferObject particleDeletionListSSBO;
	std::unordered_map<InterpolationType, float> interpolationType;
	std::vector<unsigned int> deletionList{};
	int numberOfDeletions;
	Engine& engine;
};

