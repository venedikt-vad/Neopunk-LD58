#pragma once
#include "raylib.h"
#include "raymath.h"
#include "VVADExtras.h"

struct ParticleParams {
	Texture tex;
	float scale = 1;
	float lifetime = 5;
	bool gravity = false;
	bool collisions = false;
	float bounceAmount = 0.6;
	int maxBounces = 5;

};

class Particle {
public:
	Particle();
	Particle (vec3 loc, vec3 velocity, ParticleParams* params);
	~Particle ();

	vec3 pos;
	vec3 vel;

	void Draw(Camera cam);

	void Update(float deltaTime, Model modelMap, Matrix mapMatrix);

	bool pendingDestroy = false;
private:
	ParticleParams* p;

	int bounceCount = 0;
	bool physicsFreeze = false;
	
	double spawnTime;
};