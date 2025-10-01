#pragma once
#include "raylib.h"
#include <vector>
#include <algorithm>
#include "VVADExtras.h"

class CollisionManager {
public:
	CollisionManager (Mesh MapCollision, Transform MapTransform);
	~CollisionManager ();

	RayCollision GetRayCollision(Ray ray, bool skipDynamicBoxes = false);

	SphereTraceCollision GetSphereCollision(Ray ray, float sphereRadius, bool skipDynamicBoxes = false);

	int AddDynamicBox(Transform transform);
	void UpdateDynamicBox(int index, Transform transform);

	void DrawBox(int index, Material m);

	Mesh MapStaticMesh;
	Mesh UnitBoxMesh;
private:
	Matrix MapMeshTransform;
	std::vector<Matrix> DynamicBoxes;
};

