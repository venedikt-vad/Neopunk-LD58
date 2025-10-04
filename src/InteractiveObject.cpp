#include "InteractiveObject.h"
#include "Collision/CollisionManager.h"
#include <string>

void InteractiveObject::Interactive() {
	PlayerFP& player = PlayerFP::Instance();
	Ray rayCameraPlayer = player.CameraRay();
	Vector3 rangeVector = rayCameraPlayer.position - objectTransform.translation;
	rangeToObject = Vector3Length(rangeVector);
	if (rangeToObject < 5
		&& (Vector3DotProduct(Vector3Normalize(rayCameraPlayer.position - rayCameraPlayer.direction), Vector3Normalize(rangeVector)) > 0.8)) {
		if (IsKeyDown(usedKey)) {
			ResultInteract();
		}
	}
}

void InteractiveObject::ResultInteract() {
	//TODO
}

void InteractiveObject::Update(float dt) {
	Interactive();
}

InteractiveObject::InteractiveObject() {

	int usedKey = KEY_NULL;
	rangeToObject = std::numeric_limits<float>::infinity();;

	PlayerFP& player = PlayerFP::Instance();
	Ray rayCameraPlayer = player.CameraRay();

}

InteractiveObject::~InteractiveObject() {

}
