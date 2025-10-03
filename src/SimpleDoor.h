#pragma once
#include "raylib.h"
#include "raymath.h"
#include "VVADExtras.h"
#include "Collision\CollisionManager.h"

class SimpleDoor {
public:
	SimpleDoor (Transform DoorTransform, CollisionManager* collisionManager);
	~SimpleDoor ();


	void Update(float dt);
	void Draw(Material m);

private:
	CollisionManager* cMngr;
	CollisionBox collider;
	Transform t;

};
