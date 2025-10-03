#include "SimpleDoor.h"

using namespace VLV;

SimpleDoor::SimpleDoor(Transform doorTransform, CollisionManager* collisionManager) {
	collider = NewCollider(doorTransform);
	collisionManager->AddDynamicBox(&collider);
	t = doorTransform;
	cMngr = collisionManager;
	sound = new Sound3d("resources/sounds/openTheDoor.mp3");
	sound->Play();
}

SimpleDoor::~SimpleDoor() {
	Transform newT = t;
	newT.translation.z -= 100;
	collider.isPendingRemove = true;
	delete sound;
}

void SimpleDoor::Update(float dt) {
	Transform newT = t;
	newT.translation.z += (sin(GetTime() / 2) + 1);
	collider.transform = newT;
	// if (!sound->IsPlayingSound()) {
	// 	sound->Play();
	// }
	sound->SetSoundPosition(playerInstance.camera, newT.translation);
}

void SimpleDoor::Draw(Material m) {
	cMngr->DrawBox(&collider, m);
}
