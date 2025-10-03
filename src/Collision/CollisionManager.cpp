#include "CollisionManager.h"
#include "CollisionManager.h"

CollisionManager::CollisionManager() {
}

CollisionManager::CollisionManager(Mesh MapCollision, Transform MapTransform) {
	MapStaticMesh = MapCollision;
	MapMeshTransform = TransformToMatrix(MapTransform);

	Model boxModel = LoadModel("resources/UnitCube.obj");
	UnitBoxMesh = boxModel.meshes[0];
}

CollisionManager ::~CollisionManager() {

}

void CollisionManager::SetMapCollision(Mesh MapCollision, Transform MapTransform) {
	MapStaticMesh = MapCollision;
	MapMeshTransform = TransformToMatrix(MapTransform);
}

RayCollision CollisionManager::GetRayCollision(Ray ray, bool skipDynamicBoxes) {
	RayCollision mapCollisionData = GetRayCollisionMesh(ray, MapStaticMesh, MapMeshTransform);
	if(skipDynamicBoxes) return mapCollisionData;
	for (size_t i = 0; i < DynamicBoxes.size(); i++) {
		if (DynamicBoxes[i]->isPendingRemove) {
			DynamicBoxes.erase(DynamicBoxes.begin()+i);
			i--;
		}
		RayCollision boxCollisionData = GetRayCollisionMesh(ray, UnitBoxMesh, TransformToMatrix(DynamicBoxes[i]->transform));
		if (boxCollisionData.hit && ((boxCollisionData.distance < mapCollisionData.distance) || !mapCollisionData.hit)) mapCollisionData = boxCollisionData;
	}
	return mapCollisionData;
}

SphereTraceCollision CollisionManager::GetSphereCollision(Ray ray, float sphereRadius, bool skipDynamicBoxes) {
	SphereTraceCollision mapCollisionData = GetSphereTraceCollisionMesh(ray, sphereRadius, MapStaticMesh, MapMeshTransform);
	if (skipDynamicBoxes) return mapCollisionData;
	for (size_t i = 0; i < DynamicBoxes.size(); i++) {
		if (DynamicBoxes[i]->isPendingRemove) {
			DynamicBoxes.erase(DynamicBoxes.begin() + i);
			i--;
		}
		SphereTraceCollision boxCollisionData = GetSphereTraceCollisionMesh(ray, sphereRadius, UnitBoxMesh, TransformToMatrix(DynamicBoxes[i]->transform));
		if (boxCollisionData.hit && ((boxCollisionData.distance < mapCollisionData.distance) || !mapCollisionData.hit)) mapCollisionData = boxCollisionData;
	}
	return mapCollisionData;
}

void CollisionManager::AddDynamicBox(CollisionBox* box) {
	DynamicBoxes.push_back(box);
}


void CollisionManager::DrawBox(CollisionBox* box, Material m) {

	DrawMesh(UnitBoxMesh, m, TransformToMatrix(box->transform));
}

CollisionBox NewCollider(Transform t) {
	return { t, false };
}
