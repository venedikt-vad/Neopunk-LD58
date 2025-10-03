#pragma once
#include "raylib.h"
#include "Object.h"
#include <vector>
#include <string>
#include "Collision\CollisionManager.h"

namespace VLV {
	class ObjectManager {
	public:

		void Append();

		void Delete();

		static ObjectManager& Instance();



	private:

		ObjectManager() {}

		~ObjectManager() {}

		ObjectManager(ObjectManager const&) = delete;

		ObjectManager& operator= (ObjectManager const&) = delete;

		std::vector<Object*> objects;


	};
}