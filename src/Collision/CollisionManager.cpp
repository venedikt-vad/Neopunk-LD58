#include "CollisionManager.h"

CollisionManager::CollisionManager(Mesh MapCollision, Transform MapTransform) {
	MapStaticMesh = MapCollision;
	MapMeshTransform = TransformToMatrix(MapTransform);

	Model boxModel = LoadModel("resources/UnitCube.obj");
	UnitBoxMesh = boxModel.meshes[0];
}

CollisionManager ::~CollisionManager() {

}

RayCollision CollisionManager::GetRayCollision(Ray ray, bool skipDynamicBoxes) {
	RayCollision mapCollisionData = GetRayCollisionMesh(ray, MapStaticMesh, MapMeshTransform);
	if(skipDynamicBoxes) return mapCollisionData;
	for (size_t i = 0; i < DynamicBoxes.size(); i++) {
		RayCollision boxCollisionData = GetRayCollisionMesh(ray, UnitBoxMesh, DynamicBoxes[i]);
		if (boxCollisionData.hit && ((boxCollisionData.distance < mapCollisionData.distance) || !mapCollisionData.hit)) mapCollisionData = boxCollisionData;
	}
	return mapCollisionData;
}

SphereTraceCollision CollisionManager::GetSphereCollision(Ray ray, float sphereRadius, bool skipDynamicBoxes) {
	SphereTraceCollision mapCollisionData = GetSphereTraceCollisionMesh(ray, sphereRadius, MapStaticMesh, MapMeshTransform);
	if (skipDynamicBoxes) return mapCollisionData;
	for (size_t i = 0; i < DynamicBoxes.size(); i++) {
		SphereTraceCollision boxCollisionData = GetSphereTraceCollisionMesh(ray, sphereRadius, UnitBoxMesh, DynamicBoxes[i]);
		if (boxCollisionData.hit && ((boxCollisionData.distance < mapCollisionData.distance) || !mapCollisionData.hit)) mapCollisionData = boxCollisionData;
	}
	return mapCollisionData;
}



int CollisionManager::AddDynamicBox(Transform transform) {
	DynamicBoxes.push_back(TransformToMatrix(transform));
	return  DynamicBoxes.size()-1;
}

void CollisionManager::UpdateDynamicBox(int index, Transform transform) {
	if (DynamicBoxes.size() <= index)return;
	DynamicBoxes[index] = (TransformToMatrix(transform));
}

void CollisionManager::DrawBox(int index, Material m) {
	DrawMesh(UnitBoxMesh, m, DynamicBoxes[index]);
}
