#pragma once
#include "raylib.h"
#include "raymath.h"
#include "InteractiveObject.h"
#include "PlayerFP.h"

//Class for an object to be lifted during interaction
class PickableObject : public InteractiveObject {
public:
	void ResultInteract() override;

	PickableObject();

	PickableObject(Model model, Matrix matrix, Transform transform, int interactiveKey = KEY_E);

	~PickableObject();

	int weightObject;

};