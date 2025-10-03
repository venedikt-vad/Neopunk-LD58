#include "SimpleDoor.h"

SimpleDoor::SimpleDoor(Transform doorTransform, CollisionManager* collisionManageg) {
	cMngr = collisionManageg;
	colliderIndex = cMngr->AddDynamicBox(doorTransform);
	t = doorTransform;
}

SimpleDoor ::~SimpleDoor() {
	Transform newT = t;
	newT.translation.z -= 100;
	cMngr->UpdateDynamicBox(colliderIndex, newT);
}

void SimpleDoor::Update(float dt) {
	Transform newT = t;
	newT.translation.z += (sin(GetTime() / 2) + 1);
	cMngr->UpdateDynamicBox(colliderIndex, newT);
}

void SimpleDoor::Draw(Material m) {
	cMngr->DrawBox(colliderIndex, m);
}
