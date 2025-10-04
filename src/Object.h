#pragma once
#include "raylib.h"
#include "raymath.h"

class Object {
	public:
		Object();

		Object(Transform& tr);

		~Object();

		Transform GetTransform();

		Vector3 GetPosition();

		Vector3 GetScale();

		Quaternion GetRotation();

		virtual int GetIndex();

		virtual void SetTranform(Transform transform);
  
		virtual void SetIndex(int i);

		virtual void Update(float dt) {};

		virtual void DrawObject() {};

		virtual void HideObject() {};

	protected:

		Transform objectTransform;

		int objectIndex;
	};
