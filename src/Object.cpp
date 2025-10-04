#include "Object.h"
#include <string>

	Object::Object() {
		objectTransform = { 0 };
		objectIndex = 0;
	}

	Object::Object(Transform& tr) {
		objectTransform = tr;
		objectIndex = 0;
	}

	Object::~Object() {
		//TODO
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


