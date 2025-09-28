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

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

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

Shader sh1;
Light lights[MAX_LIGHTS] = { 0 };

Texture2D texture;



Emitter<Particle>* em1;

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

    sh1 = LoadShader(TextFormat("resources/shaders/shadowmap.vs"), TextFormat("resources/shaders/depth.fs"));
    sh1.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(sh1, "viewPos");

    modelMap = LoadModel("resources/TestMap.obj");

    texture = LoadTexture("resources/cubicmap_atlas.png");    // Load map texture

    Material mat = LoadMaterialDefault();
    mat.shader = sh1;
    modelMap.materials[0] = mat;
    modelMap.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;    // Set map diffuse texture
    mapMatrix = MakeTransformMatrix({ 0.f, 0.f, 3.f }, { 90,0,0 }, { 3,3,3 });//MatrixMultiply(MatrixMultiply(matScale, matRotation), matTranslation);

    // Ambient light level (some basic lighting)
    int ambientLoc = GetShaderLocation(sh1, "ambient");
    float ambientValues[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
    SetShaderValue(sh1, ambientLoc, ambientValues, SHADER_UNIFORM_VEC4);

    // Create lights
    
    lights[0] = CreateLight(LIGHT_SPOT, { 17,7,7. }, Vector3UnitZ*-1, Color{ 255, 250, 240, 255 }, sh1);
    lights[1] = CreateLight(LIGHT_POINT, { 2, 1, 2 }, Vector3Zero(), RED, sh1);
    lights[2] = CreateLight(LIGHT_POINT, { -2, 1, 2 }, Vector3Zero(), GREEN, sh1);
    lights[3] = CreateLight(LIGHT_POINT, { 2, 1, -2 }, Vector3Zero(), BLUE, sh1);

    ParticleParams pt1; {
        pt1.tex = texture;
        pt1.scale = 1;
        pt1.collisions = true;
        pt1.gravity = true;
        pt1.spriteOrigin = { .5,0 };
    }
    em1 = new Emitter<Particle>(pt1, { 15,0,1.5 }, { 15,0,0 }, true);
    //em1 = new Emitter<Particle>(pt1, { 15,0,1.5 }, { 15,0,0 }, false);
    //em1->spawn_count = 20;

    

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

    SetShaderValue(sh1, sh1.locs[SHADER_LOC_VECTOR_VIEW], &player->camera.position, SHADER_UNIFORM_VEC3);
    //if(IsKeyPressed(KEY_E))em1->SpawnParticles();
    em1->Update(d, modelMap, mapMatrix);

    //lights[0].position = player->camera.position;
    //lights[0].target = player->CameraRay().direction;
    //lights[0].attenuation = 4;
    //UpdateLightValues(sh1, lights[0]);

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing(); {
        ClearBackground(RAYWHITE);

        BeginMode3D(player->camera);
        BeginShaderMode(sh1); {
            DrawMesh(modelMap.meshes[0], modelMap.materials[0], mapMatrix);
            em1->Draw(player->camera);
            //DrawBillboardPro(player->camera, texture, GetTextureRectangle(texture), { 20,5,1 }, GetCameraUp(player->camera), { 1,1 }, {0,0}, 0, WHITE);
            //DrawCubeV(player->CameraRay().position + player->CameraRay().direction*0.1, { 0.001,0.001,0.001 }, RED);
        }
        EndShaderMode();
        EndMode3D();

        DrawFPS(10, 10);
        Ray camRay = player->CameraRay();
        DrawText(Vec3ToString(camRay.position).c_str(), 10, 50, 30, RED);
        DrawText(Vec3ToString(player->velocity).c_str(), 10, 80, 30, YELLOW);
    }
    EndDrawing();
    //----------------------------------------------------------------------------------
}
