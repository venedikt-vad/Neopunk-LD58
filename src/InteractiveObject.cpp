#include "InteractiveObject.h"
#include "Collision/CollisionManager.h"
#include <string>
#include <iostream>

void InteractiveObject::Interactive() {
	PlayerFP& player = PlayerFP::Instance();
	Ray rayCameraPlayer = player.CameraRay();

	Vector3 rangeVector = rayCameraPlayer.position - objectTransform.translation;
	rangeToObject = Vector3Length(rangeVector);

	if (rangeToObject < 5
		&& (Vector3DotProduct(Vector3Normalize(rayCameraPlayer.position - rayCameraPlayer.direction), Vector3Normalize(rangeVector)) > 0.8)) {
		if (IsKeyDown(KEY_E)) {
			ResultInteract();
		}
	}
}

void InteractiveObject::ResultInteract() {
	//TODO
}

void InteractiveObject::Update(float dt) {
	Interactive();
}

InteractiveObject::InteractiveObject() {

	int usedKey = KEY_NULL;
	rangeToObject = std::numeric_limits<float>::infinity();;

	PlayerFP& player = PlayerFP::Instance();
	Ray rayCameraPlayer = player.CameraRay();

	objModel = LoadModel("resources/UnitCube.obj");
	texture = LoadTexture("resources/cubicmap_atlas.png");

	mat = LoadMaterialDefault();
	objModel.materials[0] = mat;
	objModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

	objectTransform = { { 0.f, 0.f, 3.f }, QuaternionFromEuler(PI / 2,0,0), { 3,3,3 } };
	
	objMatrix = TransformToMatrix(objectTransform);
	objModel.transform = objMatrix;
}

InteractiveObject::~InteractiveObject() {
	//TODO
}

void InteractiveObject::DrawObject() {
	DrawModel(objModel, Vector3Zero(), 1.f, WHITE);
}

void InteractiveObject::HideObject() {
	//TODO
}