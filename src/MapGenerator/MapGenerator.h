#pragma once
#include "raylib.h"
#include <vector>
#include <set>
#include <string>
#include "VVADExtras.h"
#include "Lights\LightManager.h"

#include "PickableObject.h"

#define MAP_TILE_SIZE 90.f

// Define the custom comparator for vec3 (Vector3)
struct Vec3Comparator {
	bool operator()(const vec3& lhs, const vec3& rhs) const {
		if (lhs.x < rhs.x) return true;
		if (lhs.x > rhs.x) return false;
		if (lhs.y < rhs.y) return true;
		if (lhs.y > rhs.y) return false;
		if (lhs.z < rhs.z) return true;
		// If all components are equal, return false (not less than)
		return false;
	}
};

struct MeshMatrix{
	Mesh mesh;
	Matrix matrix;
};

class MapGenerator {
public:
	MapGenerator(Shader sh, LightManager* LightM);
	~MapGenerator ();

	void Generate(int size);
	void Draw();

	void DrawTile(int x, int y);

	void ClearGenData();

	MeshMatrix GetMapTileAtLocation(vec3 loc);

private:
	vec3 map_offset = { -MAP_TILE_SIZE / 2.f, -MAP_TILE_SIZE / 2.f, 0.f };

	int mapSize;
	std::vector<std::vector<int>> mapTiles;
	std::vector<Model> mapElements;
	std::vector<std::set<vec3, Vec3Comparator>> lights;

	LightManager* LM;

	std::vector<PickableObject*> pickables;
};
