#pragma once
#include "raylib.h"
#include "raymath.h"
#include "VVADExtras.h"
#include "Collision\CollisionManager.h"
#include "Sound\Sound3d.h"
#include "PlayerFP.h"

// namespace VLV
// {
	class SimpleDoor
	{
	public:
		SimpleDoor(Transform DoorTransform);
		~SimpleDoor();

		void Update(float dt);
		void Draw(Material m);

	private:
		CollisionBox collider;
		Transform t;
		Sound3d *sound;

		PlayerFP &playerInstance = PlayerFP::Instance();
	};
// }
