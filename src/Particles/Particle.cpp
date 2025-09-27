#include "Particle.h"

//#define DEBUG_PARTICLES

Particle::Particle(vec3 loc, vec3 velocity, ParticleParams* params) {
	p = params;
	pos = loc;
	vel = velocity;
	spawnTime = GetTime();
}

Particle::~Particle(){
}

void Particle::Draw(Camera cam) {
#ifdef DEBUG_PARTICLES
	DrawBillboardPro(cam, p->tex, GetTextureRectangle(p->tex), pos, p->upZ?Vector3UnitZ:GetCameraUp(cam), { p->scale,p->scale }, p->spriteOrigin * p->scale, 0, physicsFreeze?BLUE:WHITE);
#else // DEBUG_PARTICLES
	DrawBillboardPro(cam, p->tex, GetTextureRectangle(p->tex), pos, p->upZ ? Vector3UnitZ : GetCameraUp(cam), { p->scale,p->scale }, p->spriteOrigin * p->scale, 0, WHITE);
#endif

}

void Particle::Update(float deltaTime, Model modelMap, Matrix mapMatrix) {
	double t = GetTime() - spawnTime;
	
	//Lifetime check
	if (t > p->lifetime) {
		pendingDestroy = true;
		return;
	}

	//Movement
	if (!physicsFreeze) {
		if (p->gravity) vel += p->gravityVec * deltaTime;
		vec3 offset = vel * deltaTime;
		if (p->collisions) {
			Ray checkRay = { pos, Vector3Normalize(vel) };
			RayCollision collisionDataFall = GetRayCollisionMesh(checkRay, modelMap.meshes[0], mapMatrix);
			if (collisionDataFall.hit && collisionDataFall.distance <= Vector3Length(offset)) {
				
				offset = Vector3Zeros;
				pos = collisionDataFall.point + collisionDataFall.normal*(0.01);

				if (p->bounceAmount > 0 && bounceCount<p->maxBounces) {
					vel = Vector3ConeRandom(Vector3Reflect(vel, collisionDataFall.normal), 10) * (Vector3Length(vel) * p->bounceAmount);
					bounceCount++;
					if (Vector3Length(vel) < 0.001) goto freeze;
				} else {
					freeze:
					vel = Vector3Zeros;
					physicsFreeze = true;
				}
			}
		}
		pos += offset;
	}
}