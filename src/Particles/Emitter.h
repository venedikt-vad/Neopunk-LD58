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
	vec3 position;
	vec3 direction;

	ParticleParams params;

	Emitter(size_t maxParticles = 32, ParticleParams p = {0}) {
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
	};


private:
	size_t max_particles = 32;

	std::vector<ParticleType*> particles;
};

