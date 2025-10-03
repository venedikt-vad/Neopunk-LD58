#include "SimpleDoor.h"

using namespace VLV;

SimpleDoor::SimpleDoor(Transform doorTransform) {
	collider = NewCollider(doorTransform);
	CollisionManager& cMngr = CollisionManager::Instance();
	cMngr.AddDynamicBox(&collider);
	t = doorTransform;
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
	newT.translation.z += (float)(sin(GetTime() / 2.f) + 1.f);
	collider.transform = newT;
	// if (!sound->IsPlayingSound()) {
	// 	sound->Play();
	// }
	sound->SetSoundPosition(playerInstance.camera, newT.translation);
}

void SimpleDoor::Draw(Material m) {
	CollisionManager& cMngr = CollisionManager::Instance();
	cMngr.DrawBox(&collider, m);
}
