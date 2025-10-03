#include "ObjectManager.h"

void ObjectManager::Append()
{
}

void ObjectManager::Delete()
{
}

ObjectManager& ObjectManager::Instance()
	{
		static ObjectManager oM;
		return oM;
	}
