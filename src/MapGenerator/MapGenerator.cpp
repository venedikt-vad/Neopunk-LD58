#include "MapGenerator.h"


MapGenerator::MapGenerator(Shader sh) {
	Texture2D texture;
	Material mat;

    texture = LoadTexture("resources/tex/WorldTextures.png"); 
    mat = LoadMaterialDefault();
    mat.shader = sh;

    for (size_t i = 0; i < 1; i++) {
        std::string path = "resources/tileEmpty";
        path += i;
        path +=".obj";
        mapElements.push_back(LoadModel(path.c_str()));
        mapElements[mapElements.size() - 1].materials[0] = mat;
        mapElements[mapElements.size() - 1].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
    }
    

    //Transform mapTransform = { { 0.f, 0.f, 3.f }, QuaternionFromEuler(PI / 2,0,0), { 3,3,3 } };

}

MapGenerator ::~MapGenerator() {

}

void MapGenerator::Generate(int size)
{
}

void MapGenerator::Draw()
{
}
