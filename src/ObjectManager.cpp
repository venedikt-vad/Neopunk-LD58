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
		objects.erase(iter);
	} 
	else {
		throw std::string("EXEPTION: attempt to delete an item from an empty list of objects");
	}
}

void ObjectManager::Delete(int objectIndex) {
	if (!objects.empty()) {
		objects.erase(objects.begin() + objectIndex);
	}
	else {
		throw std::string("EXEPTION: attempt to delete an item from an empty list of objects");
	}
}

ObjectManager::ObjectManager() {
	//TODO
}

ObjectManager::~ObjectManager() {
	//TODO
}

ObjectManager& ObjectManager::Instance() {
		static ObjectManager oM;
		return oM;
	}

