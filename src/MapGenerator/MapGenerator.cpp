#include "MapGenerator.h"

#include "PlayerFP.h"
#include "ObjectManager.h"
#include "Collision\CollisionManager.h"
#include "Obtacles\Mine.h"

MapGenerator::MapGenerator(Shader sh, LightManager* LightM) {
    LM = LightM;
	Texture2D texture;
	Material mat;

    texture = LoadTexture("resources/tex/WorldTextures.png"); 
    mat = LoadMaterialDefault();
    mat.shader = sh;

    for (size_t i = 0; i < 5; i++) {
        std::string path = "resources/MapTiles/tileModel";
        path += std::to_string(i);
        path += ".obj";
        mapElements.push_back(LoadModel(path.c_str()));
        mapElements[mapElements.size() - 1].materials[0] = mat;
        mapElements[mapElements.size() - 1].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

        path = "resources/MapTiles/tileLights";
        path += std::to_string(i);
        path += ".obj"; 
        Model lightsM = LoadModel(path.c_str());
        std::set<vec3, Vec3Comparator> lVec;
            if (lightsM.meshes[0].vertices != NULL) {
                int vertexCount = lightsM.meshes[0].vertexCount;
                for (int i = 0; i < vertexCount; i++) {
                    int vi = i * 3;
                    vec3 v = { lightsM.meshes[0].vertices[vi],-lightsM.meshes[0].vertices[vi+2],lightsM.meshes[0].vertices[vi+1] };
                    lVec.insert(v);
                }
            }
        lights.push_back(lVec);
    }

}

MapGenerator ::~MapGenerator() {
    ClearGenData();
}

void MapGenerator::Generate(int size){
    ClearGenData();
    
    mapSize = size;

    LM_Light L{};
    

    L.type = LM_SPOT; L.enabled = LM_SIMPLE_AND_VOLUMETRIC; L.radius = 40.f; L.angle = 20.f;  L.intensity = 15;
    L.direction = Vector3UnitZ * -1;

    Model pickableM = LoadModel("resources/Hardware.obj");
    CollisionManager& cMngr = CollisionManager::Instance();

    for (size_t x = 0; x < size; x++) {
        std::vector<int> v;

        for (size_t y = 0; y < size; y++) {
            if (x == 0 || y == 0 || x == (size - 1) || y == (size - 1)) {
                v.push_back(1);
            } else if (x == 1 && y == 1) {
                v.push_back(0);
            }else {
                v.push_back(GetRandomValue(2, mapElements.size() - 1));
            }

            vec3 pos = { x * MAP_TILE_SIZE,y * MAP_TILE_SIZE,0 };
            for (vec3 v : lights[v[y]]) {
                float rndF = randomFloat0to1();
                unsigned char b = 155 + (int)(100 * randomFloat0to1());
                unsigned char rg = 155 + (int)(100 * (1 - randomFloat0to1()));
                L.color = { rg,rg,b,255 };

                L.position = v + pos;
                LM->Add(L);
            }
        }
        mapTiles.push_back(v);
    }

    vec3 vecOffs = { MAP_TILE_SIZE - 10, MAP_TILE_SIZE - 10, 0 };
    //Filling created tiles
    for (size_t x = 0; x < size; x++) {
        for (size_t y = 0; y < size; y++) {

            vec3 pos = { x * MAP_TILE_SIZE,y * MAP_TILE_SIZE,0 };

            for (size_t i = 0; i < 5; i++) {
                vec3 v = Vector3RandomInVolume({ MAP_TILE_SIZE - 20,MAP_TILE_SIZE - 20,30 }) - vecOffs;
                //vec3 v = {0,0, 30};
                Ray r = { pos + v, Vector3UnitZ*-1 };
                RayCollision c = cMngr.GetRayCollision(r, true);
                if (c.hit && (Vector3DotProduct(Vector3UnitZ, c.normal)>0.6)) {
                    pickables.push_back(new PickableObject(pickableM, { c.point, QuaternionFromEuler(PI / 2,0,0), Vector3Ones }, KEY_E));
                }

            }

            for (size_t i = 0; i < 3; i++) {
                vec3 v = Vector3RandomInVolume({ MAP_TILE_SIZE - 20,MAP_TILE_SIZE - 20,30 }) - vecOffs;
                //vec3 v = {0,0, 30};
                Ray r = { pos + v, Vector3UnitZ * -1 };
                RayCollision c = cMngr.GetRayCollision(r, true);
                if (c.hit && (Vector3DotProduct(Vector3UnitZ, c.normal) > 0.6)) {
                    Mine* mine = new Mine();
                    //c.point += {0.f, 0.f, 0.5f};
                    mine->SetTranform({ c.point, QuaternionIdentity(), Vector3Ones});
                }

            }

            
        }
    }


    PlayerFP& player = PlayerFP::Instance();
    player.position = { 1 * MAP_TILE_SIZE,1 * MAP_TILE_SIZE,1 };

}

void MapGenerator::Draw() {
    PlayerFP& player = PlayerFP::Instance();

    int tileX = Clamp(round((player.camera.position.x /*- map_offset.x*/) / MAP_TILE_SIZE), 0, mapSize-1);
    int tileY = Clamp(round((player.camera.position.y /*- map_offset.y*/) / MAP_TILE_SIZE), 0, mapSize-1);

    DrawTile(tileX, tileY);
    DrawTile(tileX-1, tileY);
    DrawTile(tileX, tileY-1);
    DrawTile(tileX+1, tileY);
    DrawTile(tileX, tileY + 1);

}

void MapGenerator::DrawTile(int x, int y) {
    x = Clamp(x, 0, mapSize - 1);
    y = Clamp(y, 0, mapSize - 1);

    vec3 pos = { (float)x,(float)y, 0 };
    pos = (pos * MAP_TILE_SIZE);
    //pos = pos + map_offset;

    DrawMesh(mapElements[mapTiles[x][y]].meshes[0], mapElements[mapTiles[x][y]].materials[0],
        TransformToMatrix({ pos, QuaternionFromEuler(PI / 2, 0, 0), {1,1,1} }));
}

void MapGenerator::ClearGenData() {
    mapTiles.clear();
    LM->Clear();

    ObjectManager& objManager = ObjectManager::Instance();
    objManager.Delete();
    pickables.clear();
}

MeshMatrix MapGenerator::GetMapTileAtLocation(vec3 loc){
    int tileX = Clamp(round((loc.x /*- map_offset.x*/) / MAP_TILE_SIZE), 0, mapSize-1);
    int tileY = Clamp(round((loc.y /*- map_offset.y*/) / MAP_TILE_SIZE), 0, mapSize-1);

    vec3 pos = { (float)tileX,(float)tileY, 0 };
    pos = (pos * MAP_TILE_SIZE);
    //pos = pos + map_offset;

    Matrix mapMatrix = TransformToMatrix({ pos, QuaternionFromEuler(PI / 2, 0, 0), {1,1,1} });

    return MeshMatrix({ mapElements[mapTiles[tileX][tileY]].meshes[0], mapMatrix });
}
