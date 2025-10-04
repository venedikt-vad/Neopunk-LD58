#pragma once
#include "raylib.h"
#include "raymath.h"
#include "Object.h"
#include "PlayerFP.h"

//Class of the interaction object, when located nearby
class InteractiveObject : Object {
public:
	void Interactive();

	//A function for redefining the outcome of an interaction in the inheritors
	virtual void ResultInteract();

	void Update(float dt) override;

	InteractiveObject();

	~InteractiveObject();


protected:

	int usedKey;

	Ray rayCameraPlayer;
	float rangeToObject;

};
