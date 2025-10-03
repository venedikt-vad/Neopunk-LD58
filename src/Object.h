#pragma once
#include "raylib.h"
#include "raymath.h"

	class Object {
	public:
		Object();

		~Object();

		Transform GetTransform();

		Vector3 GetPosition();

		Vector3 GetScale();

		Quaternion GetRotation();

		int GetIndex();

		void SetTranfarm();

		virtual void Update(float dt) {};

		virtual void DrawObject() {};

		virtual void HideObject() {};

	protected:

		Transform objectTransform;

		int objectIndex;
	};
