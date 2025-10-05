#include "ObjectManager.h"
#include <algorithm>
#include <string>

void ObjectManager::Append(Object* obj) { 
	objects.push_back(obj);
	obj->SetIndex(objects.size() - 1);
}

void ObjectManager::Delete() {
	objects.clear();
}

void ObjectManager::Delete(Object* obj) {
	if (!objects.empty()) {
		auto iter = std::find(objects.begin(), objects.end(), obj);
		if (iter == objects.end()) {
			throw std::string("EXEPTION: couldn't find an element in the array");
		}
		for (int i = iter - objects.begin(); i < objects.size(); i++) {
			objects[i]->SetIndex(i);
		}
	} 
	else {
		throw std::string("EXEPTION: attempt to delete an item from an empty list of objects");
	}
}

void ObjectManager::Delete(int objectIndex) {
	if (!objects.empty()) {
		delete (objects.at(objectIndex));

		objects.erase(objects.begin() + objectIndex);

		for (int i = objectIndex; i < objects.size(); i++) {
			objects[i]->SetIndex(i);
		}
	}
	else {
		throw std::string("EXEPTION: attempt to delete an item from an empty list of objects");
	}
}

void ObjectManager::UpdateObjects(int dt) {
	for (auto elem : objects) {
		elem->Update(dt);
	}
}

void ObjectManager::DrawObjects() {
	for (auto elem : objects) {
		elem->DrawObject();
	}
}

ObjectManager::ObjectManager() {
	//TODO
}

ObjectManager::~ObjectManager() {
	//TODO
}

//A utility function that removes an element only from ObjectManager
//void ObjectManager::__DeleteAdress(Object* obj) {
	//auto iter = std::find(objects.begin(), objects.end(), obj);

	//objects.erase(iter);
//}

int ObjectManager::GetSizeObjects() {
	return objects.size();
}

ObjectManager& ObjectManager::Instance() {
		static ObjectManager oM;
		return oM;
	}

