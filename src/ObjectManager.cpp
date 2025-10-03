#include "ObjectManager.h"

namespace VLV {
	void Append()
	{

	}

	void Delete()
	{

	}

	ObjectManager& ObjectManager::Instance()
	{
		static ObjectManager oM;
		return oM;
	}
}