/*******************************************************************************************
*
*   raylib game template
*
*   <Game title>
*   <Game description>
*
*   This game has been created using raylib (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2021 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif


#include <string>
#include <cmath>

#include "VVADExtras.h"
#include "Lights\LightManager.h"
#include "Collision\CollisionManager.h"
#include "Particles\Emitter.h"
#include "MapGenerator\MapGenerator.h"
#include <iostream>

#include "PlayerFP.h"
#include "SimpleDoor.h"

#include "Object.h"
#include "InteractiveObject.h"
#include "PickableObject.h"

#include "ObjectManager.h"

#include <rlgl.h>
#include "Enemies/EnemyTV.h"
#include "Obtacles/Laser.h"
#include "Obtacles/Mine.h"

// using namespace VLV;
//----------------------------------------------------------------------------------
// Shared Variables Definition (global)
//----------------------------------------------------------------------------------
Font font = { 0 };
Music music = { 0 };
Sound fxCoin = { 0 };

Shader sh1;
LightManager* gLightMgr = nullptr;
MapGenerator* map;


Model modelTV;

EnemyTV* enemy;

Laser* laser;
Mine* mine;

Texture2D texture;
Material mat;
Material mat1;
Material mat2;
Material mat3;

SimpleDoor* door1 = nullptr;

Emitter<Particle>* em1;
Emitter<Particle>* em2;

PickableObject* interactiveObjectTest;

static const int screenWidth = 1920;
static const int screenHeight = 1080;


static void UpdateGame(void);

//----------------------------------------------------------------------------------
// Program main entry point
//----------------------------------------------------------------------------------
int main(void) {
    // Initialization
    //---------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "raylib game");
    InitAudioDevice();      // Initialize audio device

    
    // Load global data (assets that must be available in all screens, i.e. font)
    font = LoadFont("resources/mecha.png");
    //music = LoadMusicStream("resources/ambient.ogg"); // TODO: Load music
    //fxCoin = LoadSound("resources/coin.wav");

    //SetMusicVolume(music, 1.0f);
    //PlayMusicStream(music);

    sh1 = LoadShader(TextFormat("resources/shaders/shadowmap.vs"), TextFormat("resources/shaders/depth_with_intensity.fs"));
    sh1.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(sh1, "viewPos");

    texture = LoadTexture("resources/cubicmap_atlas.png");    // Load placeholder texture
    mat = LoadMaterialDefault();
    mat.shader = sh1;

    ObjectManager& objManager = ObjectManager::Instance();
    
    //interactiveObjectTest = new PickableObject(LoadModel("resources/UnitCube.obj"), { { 0.f, 0.f, 0.f }, QuaternionFromEuler(PI / 2,0,0), { 0.1,0.1,0.1 } }, KEY_E);

    modelTV = LoadModel("resources/TV.obj");
    modelTV.transform = TransformToMatrix({ { 0.f, 0.f, 0.f }, QuaternionFromEuler(0,0,0), { 1,1,1 } });

    //enemy = new EnemyTV();
    // enemy->SetTranform({ { 0.f, 0.f, 0.f }, QuaternionFromEuler(PI/2,0,0), { 1,1,1 } });

    laser = new Laser();
    laser->SetTranform({ { 1.f, 0.f, 1.f }, QuaternionFromEuler(PI/3,PI/3,PI/3), { 1,1,1 } });

    mine = new Mine();
    mine->SetTranform({ { 10.f, 0.f, 0.f }, QuaternionFromEuler(0, 0, 0), { 1,1,1 } });
    
    // Ambient light level (some basic lighting)
    int ambientLoc = GetShaderLocation(sh1, "ambient");
    float ambientValues[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
    SetShaderValue(sh1, ambientLoc, ambientValues, SHADER_UNIFORM_VEC4);
    
    gLightMgr = new LightManager(sh1);

    map = new MapGenerator(sh1, gLightMgr);

    CollisionManager& cMngr = CollisionManager::Instance(map);

    map->Generate(6);

    ParticleParams pt1; {
        pt1.tex = texture;
        pt1.scale = 1;
        pt1.collisions = true;
        pt1.gravity = true;
        pt1.spriteOrigin = { .5,0 };
    }
    em1 = new Emitter<Particle>(pt1, { 15,3,1.5 }, { 15,0,0 }, true);
    //em1->spawn_count = 20;

    ParticleParams pt2; {
        pt2.tex = texture;
        pt2.scale = .04f;
        pt2.collisions = false;
        pt2.gravity = false;
        pt2.upZ = false;
        pt2.spriteOrigin = { 0,0 };
        pt2.lifetime = 3;
    }
    em2 = new Emitter<Particle>(pt2, { 48,6,1.5 }, { 1,0,0 }, true, 256);
    em2->cone_radius = 360;
    em2->spawnVolumeSize = { 30,30,3 };
    em2->spawn_period = 0.1f;
    em2->spawn_count = 15;
    em2->initial_velocity = 2;

    Transform doorTransform = { { 23.f, 9.f, 3.f }, QuaternionIdentity(), {1,6,2}};

    door1 = new SimpleDoor(doorTransform);

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateGame, 60, 1);
#else
    SetTargetFPS(60);       // Set our game to run at 60 frames-per-second
    while (!WindowShouldClose()) {
        UpdateGame();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------

    // Unload global data loaded
    UnloadFont(font);
    //UnloadMusicStream(music);
    //UnloadSound(fxCoin);

    UnloadTexture(texture);

    CloseAudioDevice();     // Close audio context
    CloseWindow();          // Close window and OpenGL context
    return 0;
}

bool freezeLightCooling = false;

// Update and draw game frame
static void UpdateGame(void) {
    PlayerFP& player = PlayerFP::Instance();
    CollisionManager& cMngr = CollisionManager::Instance();


    ObjectManager& objManager = ObjectManager::Instance();

    float d = GetFrameTime();
    // Update
    //----------------------------------------------------------------------------------
    //UpdateMusicStream(music);       // NOTE: Music keeps playing between screens
    player.Update(d);
    laser->Update(d);
    if (mine != nullptr) {
        mine->Update(d);
    }


    SetShaderValue(sh1, sh1.locs[SHADER_LOC_VECTOR_VIEW], &player.camera.position, SHADER_UNIFORM_VEC3);
    if(IsKeyPressed(KEY_ENTER) && IsKeyDown(KEY_LEFT_ALT)) ToggleFullscreen();
    if (IsKeyPressed(KEY_L)) freezeLightCooling = !freezeLightCooling;
    
    //em1->Update(d);
    //em2->Update(d);

    door1->Update(d);
    //lights[0].position = player->camera.position;
    //lights[0].target = player->CameraRay().direction;
    LM_Light& edit = gLightMgr->Get(0);
    edit.angle = fmodf((float)GetTime(), 1.0f) * 45.f;

    if(!freezeLightCooling)gLightMgr->SyncToGPU(player.camera);

    objManager.UpdateObjects(d);

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing(); {
        ClearBackground(BLACK);
        
        BeginMode3D(player.camera);
        BeginShaderMode(sh1); {
            map->Draw();
            //DrawMesh(modelMap.meshes[0], modelMap.materials[0], mapMatrix);
            // DrawModel(modelTV, { 0.f, 0.f, 3.f }, 1.f, WHITE);
            //em1->Draw(player.camera);

            //em2->Draw(player.camera);

            objManager.DrawObjects();

            door1->Draw(mat);

            laser->DrawObject();
            if (mine != nullptr) {
                mine->DrawObject();
            }

            //DrawBillboardPro(player->camera, texture, GetTextureRectangle(texture), { 20,5,1 }, GetCameraUp(player->camera), { 1,1 }, {0,0}, 0, WHITE);
            //DrawCubeV(player->CameraRay().position + player->CameraRay().direction*0.1, { 0.001,0.001,0.001 }, RED);
            //DrawSphere({ 44, 13, 1 }, .5, Color{ 230, 41, 55, 100 });
            //DrawSphere({ 44, 5, 1 }, .5, Color{ 230, 41, 55, 100 });
        }
        EndShaderMode();
        EndMode3D();

        DrawFPS(10, 10);
        Ray camRay = player.CameraRay();
        DrawText(Vec3ToString(player.position).c_str(), 10, 50, 30, RED);
        DrawText((FloatToString(Vector3Length(player.velocity)) + " | " + Vec3ToString(player.velocity)).c_str(), 10, 80, 30, player.isGrounded ? YELLOW : SKYBLUE);


        //HUD

        //Backpack HUD
        {
            const Color backpackCol = Color{ 200, 189, 0, 200 };
            const Rectangle backpackRec = { 10, screenHeight - 210, 20, 200 };

            DrawText("backpack", 40, screenHeight - 80, 20, backpackCol);
            DrawText((std::to_string(player.invetoryWeight) + " kg").c_str(), 40, screenHeight - 60, 50, backpackCol);
            DrawRectangleLinesEx(backpackRec, 5, backpackCol);
            DrawRectangle(backpackRec.x,
                backpackRec.y + (backpackRec.height * (1-player.getBackpackPercent())),
                backpackRec.width, backpackRec.height * player.getBackpackPercent(),
                backpackCol);
        }
        

        //screenWidth
        //screenHeight
    }
    EndDrawing();
    //----------------------------------------------------------------------------------
}
