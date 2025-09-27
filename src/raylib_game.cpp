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
#include "VVADExtras.h"
#include "PlayerFP.h"

#include "Particles\Emitter.h"
//----------------------------------------------------------------------------------
// Shared Variables Definition (global)
//----------------------------------------------------------------------------------
Font font = { 0 };
Music music = { 0 };
Sound fxCoin = { 0 };

PlayerFP* player = nullptr;

Model modelMap;
Matrix mapMatrix;
Texture2D texture;


Emitter<Particle>* em1;

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------
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

    player = new PlayerFP();


    modelMap = LoadModel("resources/TestMap.obj");

    texture = LoadTexture("resources/cubicmap_atlas.png");    // Load map texture
    Material mat = LoadMaterialDefault();
    SetModelMeshMaterial(&modelMap, 0, mat.shader.id);
    
    modelMap.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;    // Set map diffuse texture
    mapMatrix = MakeTransformMatrix({ 0.f, 0.f, 3.f }, { 90,0,0 }, { 3,3,3 });//MatrixMultiply(MatrixMultiply(matScale, matRotation), matTranslation);

    ParticleParams pt1; {
        pt1.tex = texture;
        pt1.scale = 1;
        pt1.collisions = true;
        pt1.gravity = true;
    }
    
    em1 = new Emitter<Particle>(32, pt1);

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

// Update and draw game frame
static void UpdateGame(void) {
    float d = GetFrameTime();
    // Update
    //----------------------------------------------------------------------------------
    //UpdateMusicStream(music);       // NOTE: Music keeps playing between screens
    if (!player)return;
    player->Update(d, modelMap,mapMatrix);

    if(IsKeyPressed(KEY_E))em1->SpawnParticle({ 15,0,1.5 }, { 15,0,0 });
    em1->Update(d, modelMap, mapMatrix);
    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing(); {
        ClearBackground(RAYWHITE);

        BeginMode3D(player->camera); {
            DrawMesh(modelMap.meshes[0], modelMap.materials[0], mapMatrix);
            em1->Draw(player->camera);
            DrawBillboardPro(player->camera, texture, GetTextureRectangle(texture), { 20,5,1 }, GetCameraUp(player->camera), { 1,1 }, {0,0}, 0, WHITE);
            //DrawCubeV(player->CameraRay().position + player->CameraRay().direction*0.1, { 0.001,0.001,0.001 }, RED);
        }
        EndMode3D();

        DrawFPS(10, 10);
        Ray camRay = player->CameraRay();
        DrawText(Vec3ToString(camRay.position).c_str(), 10, 50, 30, RED);
    }
    EndDrawing();
    //----------------------------------------------------------------------------------
}
