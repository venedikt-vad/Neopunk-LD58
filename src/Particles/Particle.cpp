#include "Particle.h"

Particle::Particle(vec3 loc, vec3 velocity, ParticleParams* params) {
	p = params;
	pos = loc;
	vel = velocity;
	spawnTime = GetTime();
}

Particle::~Particle(){
}

void Particle::Draw(Camera cam) {
	//DrawBillboard(cam, p->tex, pos, p->scale, WHITE);
	DrawBillboardPro(cam, p->tex, GetTextureRectangle(p->tex), pos, GetCameraUp(cam), { p->scale,p->scale }, { p->scale/2.f,p->scale/2.f}, 0, WHITE);

}

void Particle::Update(float deltaTime, Model modelMap, Matrix mapMatrix) {
	double t = GetTime() - spawnTime;
	if (t > p->lifetime) {
		pendingDestroy = true;
		return;
	}

	if (!physicsFreeze) {
		if (p->gravity) vel += Vector3UnitZ * -9.4 * deltaTime;
		vec3 offset = vel * deltaTime;
		if (p->collisions) {
			Ray checkRay = { pos, Vector3Normalize(vel) };
			RayCollision collisionDataFall = GetRayCollisionMesh(checkRay, modelMap.meshes[0], mapMatrix);
			if (collisionDataFall.hit && collisionDataFall.distance <= Vector3Length(offset)) {
				
				offset = Vector3Zeros;
				pos = collisionDataFall.point + collisionDataFall.normal*0.01;

				if (p->bounceAmount > 0 && bounceCount<p->maxBounces) {
					vel = Vector3Reflect(vel, collisionDataFall.normal) * p->bounceAmount;
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