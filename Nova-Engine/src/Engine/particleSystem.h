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
	ENGINE_DLL_API std::vector<PointLightData> getParticleLights(int count);
	ENGINE_DLL_API BufferObject const& getParticeVerticesBO();
public:
	void reset();
	void update(float dt);
public:
	std::vector<TypedResourceID<Texture>> usedTextures;
	std::vector<int> counterInEachTextures;

	// Edit as you wish
	const int MAX_PARTICLESPERTEXTURE = 20000;
	const int MAX_TEXTURETYPES = 64;
private:
	// Helper
	int getMaxParticles();
	// Spawning
	void continuousGeneration(Transform const& transform, ParticleEmitter& emitter, float dt);
	void burstGeneration(Transform const& transform, ParticleEmitter& emitter, float dt);
	void trailGeneration(Transform& transform, ParticleEmitter& emitter);
	void spawnParticle(Transform const& transform, ParticleEmitter& emitter);
	// Setup everything the particle needs to work
	void determineParticleSpawnDetails(ParticleLifespanData& particleLifeSpanData, ParticleVertex& particleVertex,glm::vec3 position, ParticleEmitter& emitter ,ParticleEmissionTypeSelection::EmissionShape emissionShape);
	void determineParticleColor(ParticleLifespanData& particleLifeSpanData, ParticleVertex& particleVertex, ParticleEmitter& emitter, float emissiveMultiplier, ColorA startcolor, ColorA endColor, glm::vec3 colorOffsetMin, glm::vec3 colorOffsetMax);
	void determineParticleSize(ParticleLifespanData& particleLifeSpanData, ParticleVertex& particleVertex, ParticleEmitter& emitter, float startSize, float endSize, float minStartSizeOffset, float maxStartSizeOffset);
	// Rotate Particle's position, velocity and force values based on transform
	void rotateParticle(ParticleLifespanData& particleLifeSpanData, ParticleVertex& particleVertex, Transform const& transform);
	// Add to compute Shader
	void addParticleToList(ParticleLifespanData& particleLifeSpanData, ParticleVertex& particleVertex, TypedResourceID<Texture> texture);
private:
	ComputeShader particleUpdateComputeShader;
	ComputeShader particleFindLightsComputeShader;
	ComputeShader particleResetAllComputeShader;
	BufferObject particleVerticesBO;
	BufferObject particlesSSBO;
	BufferObject particleLightsSSBO;
	std::unordered_map<InterpolationType, float> interpolationType;
	Engine& engine;
};

