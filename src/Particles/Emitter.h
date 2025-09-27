#pragma once
#include "raylib.h"
#include "raymath.h"
#include "VVADExtras.h"
#include<vector>
#include <memory>

#include "Particle.h"

template<typename ParticleType>
class Emitter {
public:
	vec3 position = Vector3Zeros;
	vec3 direction = Vector3UnitX;
	float cone_radius = 15;
	float initial_velocity = 2;

	float spawn_period = 0.3;

	ParticleParams params;

	Emitter(ParticleParams p = {0}, vec3 pos = Vector3Zeros, vec3 dir = Vector3UnitZ, size_t maxParticles = 64) {
		position = pos;
		direction = Vector3Normalize(dir);
		if (abs(Vector3Length(dir)-1)>0.01) initial_velocity = Vector3Length(dir);
		params = p;
		max_particles = maxParticles;
		particles.reserve(max_particles);
	};

	~Emitter() {
		for (ParticleType* pt : particles) {
			if (pt) {
				pt->~Particle();
				delete pt;
			}
		}
	};

	void Update(float deltaTime, Model modelMap, Matrix mapMatrix) {
		if ((GetTime()-last_spawn_t >= spawn_period) && (particles.size() < max_particles)) {
			SpawnParticle(position, Vector3ConeRandom(direction, cone_radius) * initial_velocity);
		}

		if (particles.size() <= 0)return;
		for (int i = particles.size(); i--; i>=0) {
			ParticleType* pt = particles[i];
			if (pt) {
				pt->Update(deltaTime, modelMap, mapMatrix);
				if (pt->pendingDestroy)particles.erase(particles.begin() + i);
			}
		}
	};
	
	void Draw(Camera cam) {
		for (ParticleType* pt : particles) {
			if (pt) {
				pt->Draw(cam);
			}
		}
	};

	void SpawnParticle(vec3 loc, vec3 dir) {
		if (particles.size() >= max_particles) return;
		ParticleType* newParticle = new ParticleType(loc, dir, &params);
		particles.push_back(newParticle);
		last_spawn_t = GetTime();
	};


private:
	size_t max_particles = 32;
	float last_spawn_t = -100;
	std::vector<ParticleType*> particles;
};

