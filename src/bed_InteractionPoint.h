#pragma once
#include "raylib.h"
#include "raymath.h"
#include "InteractiveObject.h"
#include "HQ_InteractionPoint.h"

//Class for an object to be lifted during interaction
class bed_InteractionPoint : public InteractiveObject {
public:
	void ResultInteract() override;

	bed_InteractionPoint();

	bed_InteractionPoint(HQ_InteractionPoint* HQpoint, Transform transform, int interactiveKey = KEY_E);

	~bed_InteractionPoint();

private:
	HQ_InteractionPoint* HQ;
};