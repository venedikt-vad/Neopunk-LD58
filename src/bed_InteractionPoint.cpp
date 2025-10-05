#include "bed_InteractionPoint.h"
#include "Collision/CollisionManager.h"
#include "ObjectManager.h"
#include "Object.h"
#include "PlayerFP.h"

#include <string>


void bed_InteractionPoint::ResultInteract() {
	PlayerFP& player = PlayerFP::Instance();

	if (HQ->isQuotaComplete) {
		//END_DAY
	}
}

bed_InteractionPoint::bed_InteractionPoint() {

}

bed_InteractionPoint::bed_InteractionPoint(HQ_InteractionPoint* HQpoint, Transform transform, int interactiveKey) : InteractiveObject({0}, transform, interactiveKey, false) {
	//TODO
	HQ = HQpoint;
}

bed_InteractionPoint::~bed_InteractionPoint() {
	//TODO
}

