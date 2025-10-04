#include "PickableObject.h"
#include "Collision/CollisionManager.h"
#include "ObjectManager.h"
#include "Object.h"

#include <string>


void PickableObject::ResultInteract() {
	ObjectManager& objManager = ObjectManager::Instance();
	PlayerFP& player = PlayerFP::Instance();

	player.AddObjToInventory(weightObject);

	objManager.Delete(this->GetIndex());

}

PickableObject::PickableObject() {
	weightObject = 1;
}

PickableObject::PickableObject(Model model, Matrix matrix, Transform transform, int interactiveKey) : InteractiveObject(model, matrix, transform, interactiveKey) {
	//TODO
}

PickableObject::~PickableObject() {
	//TODO
}
