#include "InteractiveObject.h"
#include "Collision/CollisionManager.h"
#include <string>

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

	objModel.materials[0] = LoadMaterialDefault();
	objModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTexture("resources/cubicmap_atlas.png");

	objectTransform = { { 0.f, 0.f, 3.f }, QuaternionFromEuler(PI / 2,0,0), { 3,3,3 } };
	
	objMatrix = TransformToMatrix(objectTransform);
	objModel.transform = objMatrix;
}

InteractiveObject::InteractiveObject(Model model, Matrix matrix, Transform transform, int interactiveKey) : Object(transform)  {
	
	int usedKey = interactiveKey;
	rangeToObject = std::numeric_limits<float>::infinity();;

	PlayerFP& player = PlayerFP::Instance();
	Ray rayCameraPlayer = player.CameraRay();
	
	objModel = model;
	objMatrix = matrix;
	

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