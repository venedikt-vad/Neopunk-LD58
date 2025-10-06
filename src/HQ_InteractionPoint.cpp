#include "HQ_InteractionPoint.h"
#include "Collision/CollisionManager.h"
#include "ObjectManager.h"
#include "Object.h"
#include "PlayerFP.h"

#include <string>


void HQ_InteractionPoint::ResultInteract() {
	PlayerFP& player = PlayerFP::Instance();

	collectedQuota += player.invetoryWeight;
	player.invetoryWeight = 0;

	if (collectedQuota >= quota) {
		isQuotaComplete = true;
		player.invetoryWeight = collectedQuota - quota;
		collectedQuota = quota;
	} else {
		player.invetoryWeight = 0;
	}
}

HQ_InteractionPoint::HQ_InteractionPoint() {
	NewQuota();
}

HQ_InteractionPoint::HQ_InteractionPoint(Transform transform, int interactiveKey) : InteractiveObject({0}, transform, interactiveKey, false) {
	NewQuota();
}

HQ_InteractionPoint::~HQ_InteractionPoint() {
	//TODO
}

void HQ_InteractionPoint::NewQuota() {
	PlayerFP& player = PlayerFP::Instance();

	quota = GetRandomValue(5,7) * player.day;
	isQuotaComplete = false;
	collectedQuota = 0;
}
