#include "InteractiveObject.h"
#include "Collision/CollisionManager.h"
#include <string>
#include "PlayerFP.h"

void InteractiveObject::Interactive() {
	PlayerFP& player = PlayerFP::Instance();
	Ray rayCameraPlayer = player.CameraRay();

	Vector3 rangeVector = rayCameraPlayer.position - objectTransform.translation;
	rangeToObject = Vector3Length(rangeVector);

	if (rangeToObject < 5
		&& (Vector3DotProduct(Vector3Normalize(rayCameraPlayer.direction*-1), Vector3Normalize(rangeVector)) > 0.8)) {
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

	objModel.materials[0] = LoadMaterialDefault();
	objModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTexture("resources/cubicmap_atlas.png");

	objectTransform = { { 0.f, 0.f, 3.f }, QuaternionFromEuler(PI / 2,0,0), { 1,1,1 } };
	
	//objMatrix = TransformToMatrix(objectTransform);
	objModel.transform = TransformToMatrix(objectTransform);
}

InteractiveObject::InteractiveObject(Model model, Transform transform, int interactiveKey, bool hasModel) : Object(transform)  {
	int usedKey = interactiveKey;
	rangeToObject = std::numeric_limits<float>::infinity();;

	PlayerFP& player = PlayerFP::Instance();
	Ray rayCameraPlayer = player.CameraRay();
	
	drawModel = hasModel;
	if(drawModel)objModel = model;
}

InteractiveObject::~InteractiveObject() {
	//TODO
}

void InteractiveObject::DrawObject() {
	if (drawModel) {
		objModel.transform = TransformToMatrix(objectTransform);
		DrawModel(objModel, Vector3Zero(), 1.f, WHITE);
	}
	
}

void InteractiveObject::HideObject() {
	//TODO
}