#pragma once
#include "raylib.h"
#include <vector>
#include <string>

#define MAP_TILE_SIZE



class MapGenerator {
public:
	MapGenerator(Shader sh);
	~MapGenerator ();

	void Generate(int size);
	void Draw();

	void ClearGenData();

private:


	int mapSize;
	std::vector<std::vector<int>> mapTiles;
	std::vector<Model> mapElements;
};
