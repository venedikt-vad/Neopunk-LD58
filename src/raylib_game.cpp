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
#include <iostream>

#include "PlayerFP.h"
#include "SimpleDoor.h"

#include "Object.h"
#include "InteractiveObject.h"
#include "PickableObject.h"

#include "ObjectManager.h"

#include <rlgl.h>
#include "Enemies/EnemyTV.h"

// using namespace VLV;
//----------------------------------------------------------------------------------
// Shared Variables Definition (global)
//----------------------------------------------------------------------------------
Font font = { 0 };
Music music = { 0 };
Sound fxCoin = { 0 };


Model modelMap;
Model modelTV;
Matrix mapMatrix;

Shader sh1;
LightManager* gLightMgr = nullptr;

EnemyTV* enemy;


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

    ObjectManager& objManager = ObjectManager::Instance();

    interactiveObjectTest = new PickableObject(LoadModel("resources/UnitCube.obj"), TransformToMatrix({ { 0.f, 0.f, 3.f }, QuaternionFromEuler(PI / 2,0,0), { 3,3,3 } }), { { 0.f, 0.f, 3.f }, QuaternionFromEuler(PI / 2,0,0), { 3,3,3 } }, KEY_E);
    

    sh1 = LoadShader(TextFormat("resources/shaders/shadowmap.vs"), TextFormat("resources/shaders/depth_with_intensity.fs"));
    sh1.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(sh1, "viewPos");
    

    modelMap = LoadModel("resources/TestMap.obj");
    modelTV = LoadModel("resources/TV.obj");
    modelTV.transform = TransformToMatrix({ { 0.f, 0.f, 0.f }, QuaternionFromEuler(PI/2,0,0), { 1,1,1 } });

    enemy = new EnemyTV();
    // enemy->SetTranform({ { 0.f, 0.f, 0.f }, QuaternionFromEuler(PI/2,0,0), { 1,1,1 } });

    texture = LoadTexture("resources/cubicmap_atlas.png");    // Load map texture

    mat = LoadMaterialDefault();
    mat.shader = sh1;
    modelMap.materials[0] = mat;
    modelMap.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;    // Set map diffuse texture

    Transform mapTransform = { { 0.f, 0.f, 3.f }, QuaternionFromEuler(PI/2,0,0), { 3,3,3 } };
    
    CollisionManager& cMngr = CollisionManager::Instance(modelMap.meshes[0], mapTransform);

    //cMngr = new CollisionManager(modelMap.meshes[0], mapTransform);

    mapMatrix = TransformToMatrix(mapTransform);//MakeTransformMatrix({ 0.f, 0.f, 3.f }, { 90,0,0 }, { 3,3,3 });//MatrixMultiply(MatrixMultiply(matScale, matRotation), matTranslation);
    // Ambient light level (some basic lighting)
    int ambientLoc = GetShaderLocation(sh1, "ambient");
    float ambientValues[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    SetShaderValue(sh1, ambientLoc, ambientValues, SHADER_UNIFORM_VEC4);
    gLightMgr = new LightManager(sh1);
    // Create lights
    
    LM_Light L{};
    L.type = LM_SPOT; L.enabled = LM_SIMPLE; L.radius = 15.f; L.angle = 45.f; L.color = { 255,250,240,255 }; L.intensity = 15;
    L.position = { 17,7,7 }; L.direction = Vector3Scale(Vector3UnitZ, -1); gLightMgr->Add(L);

    L.enabled = LM_SIMPLE;

    for (size_t i = 0; i < 30; i++) {
        L.type = LM_SPOT; L.enabled = LM_SIMPLE_AND_VOLUMETRIC; L.radius = 15.f; L.angle = 20.f; L.color = { 255,255,255,255 }; L.intensity = 15;

        L.position = { 19+i*4.f,6,11 }; gLightMgr->Add(L);

    }

    for (size_t i = 0; i < 30; i++) {
        L.type = LM_SPOT; L.enabled = LM_SIMPLE_AND_VOLUMETRIC; L.radius = 11; L.angle = 30.f; L.color = { 255,255,150,255 };

        L.position = { 12 + i * 6.f,-26,31 }; gLightMgr->Add(L);

    }


    // points:
    LM_Light P{};
    P.type = LM_POINT; P.enabled = LM_SIMPLE;
    P.position = { 2,1,2 };   P.radius = 1.f; P.color = RED;   gLightMgr->Add(P);
    P.position = { -2,1,2 };  P.radius = 3.f; P.color = GREEN; gLightMgr->Add(P);
    P.position = { 2,1,2 };   P.radius = 5.f; P.color = BLUE;  gLightMgr->Add(P);


    ParticleParams pt1; {
        pt1.tex = texture;
        pt1.scale = 1;
        pt1.collisions = true;
        pt1.gravity = true;
        pt1.spriteOrigin = { .5,0 };
    }
    em1 = new Emitter<Particle>(pt1, { 15,3,1.5 }, { 15,0,0 }, true);


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

    //em1 = new Emitter<Particle>(pt1, { 15,0,1.5 }, { 15,0,0 }, false);
    //em1->spawn_count = 20;
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

    SetShaderValue(sh1, sh1.locs[SHADER_LOC_VECTOR_VIEW], &player.camera.position, SHADER_UNIFORM_VEC3);
    //if(IsKeyPressed(KEY_E))em1->SpawnParticles();
    if(IsKeyPressed(KEY_ENTER) && IsKeyDown(KEY_LEFT_ALT)) ToggleFullscreen();
    if (IsKeyPressed(KEY_L)) freezeLightCooling = !freezeLightCooling;
    
    em1->Update(d);
    em2->Update(d);

    door1->Update(d);
    //lights[0].position = player->camera.position;
    //lights[0].target = player->CameraRay().direction;
    LM_Light& edit = gLightMgr->Get(0);
    edit.angle = fmodf((float)GetTime(), 1.0f) * 45.f;

    if(!freezeLightCooling)gLightMgr->SyncToGPU(player.camera);
    //UpdateLightsArray(sh1, lights, player->camera);

    enemy->Update(1);
    objManager.UpdateObjects(d);

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing(); {
        ClearBackground(BLACK);
        
        BeginMode3D(player.camera);
        BeginShaderMode(sh1); {
            DrawMesh(modelMap.meshes[0], modelMap.materials[0], mapMatrix);
            // DrawModel(modelTV, { 0.f, 0.f, 3.f }, 1.f, WHITE);
            em1->Draw(player.camera);

            em2->Draw(player.camera);

            objManager.DrawObjects();

            door1->Draw(mat);

            enemy->DrawObject();

            /*Ray gravRay = { { 48, -2, 2 }, {0,0,-1} };
            SphereTraceCollision gravCollision = cMngr->GetSphereCollision(gravRay, .1f);
            DrawSphere(gravRay.position, .1f, Color{ 230, 41, 55, 255 });
            DrawSphere(gravCollision.point, .1f, Color{ 0, 231, 55, 255 });*/


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
    }
    EndDrawing();
    //----------------------------------------------------------------------------------
}
