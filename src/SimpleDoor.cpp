#include "SimpleDoor.h"

SimpleDoor::SimpleDoor(Transform doorTransform, CollisionManager* collisionManager) {
	collider = NewCollider(doorTransform);
	collisionManager->AddDynamicBox(&collider);
	t = doorTransform;
	cMngr = collisionManager;
}

SimpleDoor ::~SimpleDoor() {
	Transform newT = t;
	newT.translation.z -= 100;
	collider.isPendingRemove = true;
}

void SimpleDoor::Update(float dt) {
	Transform newT = t;
	newT.translation.z += (sin(GetTime() / 2) + 1);
	collider.transform = newT;
}

void SimpleDoor::Draw(Material m) {
	cMngr->DrawBox(&collider, m);
}
