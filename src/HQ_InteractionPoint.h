#pragma once
#include "raylib.h"
#include "raymath.h"
#include "InteractiveObject.h"

//Class for an object to be lifted during interaction
class HQ_InteractionPoint : public InteractiveObject {
public:
	void ResultInteract() override;

	HQ_InteractionPoint();

	HQ_InteractionPoint(Transform transform, int interactiveKey = KEY_E);

	~HQ_InteractionPoint();

	int quota = 10;
	bool isQuotaComplete = true;
	int collectedQuota = 0;

	void NewQuota();
	

};