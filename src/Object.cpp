#include "Object.h"

	Object::Object() {
		objectTransform = { 0 };
		objectIndex = 0;
	}

	Object::~Object()
	{
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

	void Object::SetTranfarm()
	{
	}

