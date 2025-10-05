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
	}
}

HQ_InteractionPoint::HQ_InteractionPoint() {

}

HQ_InteractionPoint::HQ_InteractionPoint(Transform transform, int interactiveKey) : InteractiveObject({0}, transform, interactiveKey, false) {
	//TODO
}

HQ_InteractionPoint::~HQ_InteractionPoint() {
	//TODO
}

void HQ_InteractionPoint::NewQuota() {
	quota = GetRandomValue(12,20);
	isQuotaComplete = false;
	collectedQuota = 0;
}
