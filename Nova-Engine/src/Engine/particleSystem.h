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
	void reset();
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
	// Setup everything the particle needs to work
	void determineParticleSpawnDetails(glm::vec3 position, ParticleEmitter& emitter,ParticleLifespanData& particleLifeSpanData ,ParticleEmissionTypeSelection::EmissionShape emissionShape);
	void determineParticleColor(ParticleLifespanData& particleLifeSpanData, ParticleEmitter& emitter, float emissiveMultiplier, ColorA startcolor, ColorA endColor, glm::vec3 colorOffsetMin, glm::vec3 colorOffsetMax);
	void determineParticleSize(ParticleLifespanData& particleLifeSpanData, ParticleEmitter& emitter, float startSize, float endSize, float minStartSizeOffset, float maxStartSizeOffset);
	// Rotate Particle's position, velocity and force values based on transform
	void rotateParticle(Transform const& transform, ParticleLifespanData& particleLifeSpanData);
private:
	ComputeShader particleUpdateComputeShader;
	BufferObject particleSSBO;
	BufferObject particleDeletionListSSBO;
	std::unordered_map<InterpolationType, float> interpolationType;
	std::vector<unsigned int> deletionList{};
	int maxdeletionIndex;
	Engine& engine;
};

