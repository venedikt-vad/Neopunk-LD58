#include "Object.h"
#include "ObjectManager.h"
#include <string>

	Object::Object() {
		objectTransform = { 0 };

		ObjectManager& objManager = ObjectManager::Instance();
		objManager.Append(this);
	}

	Object::Object(Transform& tr) {
		objectTransform = tr;
		ObjectManager& objManager = ObjectManager::Instance();
		objManager.Append(this);
		objectIndex = objManager.GetSizeObjects() - 1;
	}

	Object::~Object() {
		ObjectManager& objManager = ObjectManager::Instance();
		HideObject();
		objManager.__DeleteAdress(this);
	}

	Transform Object::GetTransform() {
		return objectTransform;
	}

	Vector3 Object::GetPosition() {
		return objectTransform.translation;
	}

	Vector3 Object::GetScale() {
		return objectTransform.scale;
	}

	Quaternion Object::GetRotation() {
		return objectTransform.rotation;
	}

	int Object::GetIndex() {
		return objectIndex;
	}

	void Object::SetTranfarm(Transform& tr){
		objectTransform = tr;
	}

	void Object::SetIndex(int i) {
		objectIndex = i;
	}


