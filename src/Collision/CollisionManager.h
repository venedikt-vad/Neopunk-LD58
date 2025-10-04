#pragma once
#include "raylib.h"
#include <vector>
#include <algorithm>
#include "VVADExtras.h"
#include "MapGenerator\MapGenerator.h"

struct CollisionBox {
	Transform transform;
	bool isPendingRemove;
};

class CollisionManager {
private:
	static CollisionManager* instance;
	CollisionManager(Mesh MapCollision, Transform MapTransform);
	CollisionManager(MapGenerator* Map);
	CollisionManager();
public:
	
	static CollisionManager& Instance();
	static CollisionManager& Instance(MapGenerator* Map);
	static CollisionManager& Instance(Mesh MapCollision, Transform MapTransform);

	~CollisionManager ();

	void SetMapCollision(Mesh MapCollision, Transform MapTransform);

	RayCollision GetRayCollision(Ray ray, bool skipDynamicBoxes = false);

	SphereTraceCollision GetSphereCollision(Ray ray, float sphereRadius, bool skipDynamicBoxes = false);

	void AddDynamicBox(CollisionBox* box);

	void DrawBox(CollisionBox* box, Material m);

	Mesh MapStaticMesh;
private:

	Matrix MapMeshTransform;
	MapGenerator* map = nullptr;

	Mesh UnitBoxMesh;
	std::vector<CollisionBox*> DynamicBoxes;
};

CollisionBox NewCollider(Transform t);