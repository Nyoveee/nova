#pragma once
#include "export.h"
#include "component.h"
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
	void update(float dt);
	void emit(Transform const& transform, ParticleEmitter& emitter);
private:
	void continuousGeneration(Transform const& transform, ParticleEmitter& emitter, float dt);
	void burstGeneration(Transform const& transform, ParticleEmitter& emitter, float dt);
	void particleMovement(ParticleEmitter& emitter, float dt);
	void spawnParticle(Transform const& transform, ParticleEmitter& emitter);
private:
	Engine& engine;
};

