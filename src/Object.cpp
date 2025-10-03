#include "Object.h"

namespace VLV {

	Object::Object()
	{
		objectTransform = { 0 };
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

	void SetTranfarm() {

	}
}