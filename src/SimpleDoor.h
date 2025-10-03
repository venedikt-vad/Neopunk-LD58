#pragma once
#include "raylib.h"
#include "raymath.h"
#include "VVADExtras.h"
#include "Collision\CollisionManager.h"

class SimpleDoor {
public:
	SimpleDoor (Transform DoorTransform, CollisionManager* collisionManageg);
	~SimpleDoor ();


	void Update(float dt);
	void Draw(Material m);

private:
	CollisionManager* cMngr;
	int colliderIndex;
	Transform t;

};
