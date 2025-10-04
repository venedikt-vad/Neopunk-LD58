#pragma once
#include "raylib.h"
#include "Object.h"

#include <vector>
#include <string>

class ObjectManager {
	public:

		void Append(Object* obj);

		void Delete();

		void Delete(Object* obj);

		void Delete(int objectIndex);

		void UpdateObjects(int dt);

		void DrawObjects();

		//void __DeleteAdress(Object* obj);

		int GetSizeObjects();

		static ObjectManager& Instance();


	private:

		ObjectManager();

		~ObjectManager();

		ObjectManager(ObjectManager const&) = delete;

		ObjectManager& operator= (ObjectManager const&) = delete;

		std::vector<Object*> objects;


	};
