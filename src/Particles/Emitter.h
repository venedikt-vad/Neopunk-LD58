#pragma once
#include "raylib.h"
#include "raymath.h"
#include "VVADExtras.h"

#include<vector>
#include <memory>
#include <algorithm>
//#include <execution>

#include "Particle.h"
#include "Collision\CollisionManager.h"

template<typename ParticleType>
class Emitter {
public:
	vec3 position = Vector3Zeros;
	vec3 direction = Vector3UnitX;
	float cone_radius = 15;
	float initial_velocity = 2;

	float spawn_period = 0.3f;
	int spawn_count = 1;

	vec3 spawnVolumeSize = Vector3Zeros;

	ParticleParams params;

	bool active = true;

	Emitter(ParticleParams p = {0}, vec3 pos = Vector3Zeros, vec3 dir = Vector3UnitZ, bool autoActivate = true, size_t maxParticles = 64) {
		position = pos;
		direction = Vector3Normalize(dir);
		if (abs(Vector3Length(dir)-1)>0.01) initial_velocity = Vector3Length(dir);
		params = p;
		active = autoActivate;
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

	void Update(float deltaTime) {
		if (active) {
			if ((GetTime() - last_spawn_t >= spawn_period) && (particles.size() < max_particles)) {
				SpawnParticles();
			}
		}
		if (particles.empty()) return;

		// 1) Snapshot to iterate without mutating the vector while we loop
		//std::vector<ParticleType*> snapshot = particles;

		// 2) Parallel update
		//std::for_each(std::execution::par_unseq, snapshot.begin(), snapshot.end(),
		//	[&](ParticleType* pt) {
		//		if (pt) pt->Update(deltaTime, modelMap, mapMatrix);
		//	});

#pragma omp parallel for schedule(static)
		for (int i = 0; i < (int)particles.size(); ++i) {
			ParticleType* pt = particles[i];
			if (pt) pt->Update(deltaTime);
		}

		// 3) Single-threaded compaction + deletion of dead particles
		particles.erase(
			std::remove_if(particles.begin(), particles.end(),
				[](ParticleType* pt) {
					if (!pt) return true;
					if (pt->pendingDestroy) { delete pt; return true; }
					return false;
				}),
			particles.end()
		);
	};

	
	void Draw(Camera cam) {
		for (ParticleType* pt : particles) {
			if (pt) {
				pt->Draw(cam);
			}
		}
	};

	void SpawnParticles() {
		for (size_t i = 0; i < spawn_count; i++) {
			if (!SpawnParticle(position, Vector3ConeRandom(direction, cone_radius) * initial_velocity)) break;
		}
	}

	


private:
	size_t max_particles = 32;
	double last_spawn_t = -100;
	std::vector<ParticleType*> particles;

	bool SpawnParticle(vec3 loc, vec3 dir) {
		vec3 SpawnLoc = loc;
		if (Vector3Length(spawnVolumeSize) > 0) {
			SpawnLoc = loc + Vector3RandomInVolume(spawnVolumeSize) - (spawnVolumeSize/2.f);
		}
		if (particles.size() >= max_particles) return false;
		ParticleType* newParticle = new ParticleType(SpawnLoc, dir, &params);
		particles.push_back(newParticle);
		last_spawn_t = GetTime();

		return true;
	};
};

