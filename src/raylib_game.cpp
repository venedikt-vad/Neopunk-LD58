/*******************************************************************************************
*
*   raylib game template
*
*   NEOPUNK
*   Collect scrap in dark cyberpunk city.
*
*   This game has been created using raylib (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2021 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
//#include <rlgl.h>

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

#include <string>
#include <cmath>

#include "VVADExtras.h"
#include "Lights\LightManager.h"
#include "Collision\CollisionManager.h"
#include "ObjectManager.h"

//#include "Particles\Emitter.h"
#include "MapGenerator\MapGenerator.h"

#include "PlayerFP.h"

#include "Enemies/EnemyTV.h"
#include "HQ_InteractionPoint.h"
#include "bed_InteractionPoint.h"

Font font = { 0 };

Shader sh1;
LightManager* gLightMgr = nullptr;
MapGenerator* map;

HQ_InteractionPoint* HQ;
bed_InteractionPoint* bed;

EnemyTV* enemy;

//Texture2D noizeTexture;

static const int screenWidth = 1920;
static const int screenHeight = 1080;

static void UpdateGame(void);
static void DrawLoading();
static void GameRestart();

//----------------------------------------------------------------------------------
// Program main entry point
//----------------------------------------------------------------------------------
int main(void) {
    // Initialization
    //---------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "raylib game");
    InitAudioDevice();      // Initialize audio device

    DrawLoading();

    ToggleFullscreen();

    font = LoadFont("resources/mecha.png");
    //noizeTexture = LoadTexture("resources/noise.png");

    //Light
    {
        sh1 = LoadShader(TextFormat("resources/shaders/shadowmap.vs"), TextFormat("resources/shaders/depth_with_intensity.fs"));
        sh1.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(sh1, "viewPos");
        // Ambient light level (some basic lighting)
        int ambientLoc = GetShaderLocation(sh1, "ambient");
        float ambientValues[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
        SetShaderValue(sh1, ambientLoc, ambientValues, SHADER_UNIFORM_VEC4);
        gLightMgr = new LightManager(sh1);
    }

    ObjectManager& objManager = ObjectManager::Instance();
    
    map = new MapGenerator(sh1, gLightMgr);

    CollisionManager& cMngr = CollisionManager::Instance(map);

    PlayerFP& player = PlayerFP::Instance();

    map->Generate(8);

    HQ = new HQ_InteractionPoint({ {90 + 11.9, 90 + 26.9, 2}, QuaternionIdentity(), Vector3Ones });
    bed = new bed_InteractionPoint(HQ, { {90 + 20.3, 90 + 32.1, 2}, QuaternionIdentity(), Vector3Ones });

    enemy = new EnemyTV();
    enemy->SetTranform({ { 180.f, 180.f, 4.f }, QuaternionFromEuler(PI / 2,0,0), { 1,1,1 } });

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

    map->ClearGenData();
    UnloadFont(font);

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
    
    
    if(IsKeyPressed(KEY_ENTER) && IsKeyDown(KEY_LEFT_ALT)) ToggleFullscreen();
    
    //if (IsKeyPressed(KEY_L)) freezeLightCooling = !freezeLightCooling;  //DEBUG LIGHTS
    if (!freezeLightCooling)gLightMgr->SyncToGPU(player.camera); //Light update

    //Do not update during sleep
    if (!bed->dayEndscreen) {
        player.Update(d);
        SetShaderValue(sh1, sh1.locs[SHADER_LOC_VECTOR_VIEW], &player.camera.position, SHADER_UNIFORM_VEC3);

        objManager.UpdateObjects(d);
    }

    //Game end/restart
    if (player.hpPlayer <= 0 && IsKeyPressed(KEY_R)) {
        player.hpPlayer = 100;
        player.invetoryWeight = 0;
        player.day = 1;

        DrawLoading();
        GameRestart();
    }

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing(); {
        ClearBackground(BLACK);
        
        BeginMode3D(player.camera);
        BeginShaderMode(sh1); {
            DrawCube({ 90,90,90 }, 3000, 3000, 3, BLACK);
            map->Draw();
            objManager.DrawObjects();
        }
        EndShaderMode();
        EndMode3D();

        DrawFPS(10, 10);
        Ray camRay = player.CameraRay();

        //HUD
        {
            if (bed->dayEndscreen) {
                //Day end screen

                DrawRectangle(0, 0, screenWidth, screenHeight, BLACK);
                if (GetTime() - bed->dayEndscreenBeginTime < 3) {
                    DrawText("day finished", screenWidth / 2 - 170, screenHeight / 2 - 20, 40, WHITE);
                } else if (GetTime() - bed->dayEndscreenBeginTime < 6) {
                    DrawText("you hear the metal creaking\ncity continue to grow...", screenWidth / 2 - 230, screenHeight / 2 - 50, 40, WHITE);
                } else {
                    player.day += 1;

                    DrawLoading();
                    GameRestart();
                }
                    
            } else {
                //Player noize tex
                /*{
                    int x = GetRandomValue(0, 1000);
                    unsigned char a = player.noiseAmount * 100;
                    DrawTexture(noizeTexture, x, 0, Color{ 255, 255, 255, a });
                    DrawTexture(noizeTexture, x-1920, 0, Color{ 255, 255, 255, a });
                }*/

                //Player damage tint
                {
                    unsigned char a =  100 * (1.f - Clamp01(GetTime() - player.lastDamageTime));
                    DrawRectangle(0, 0, screenWidth, screenHeight, Color{ 230, 41, 55, a });
                }

                //Interaction HUD
                {
                    if (player.drawInteraction) {
                        const Color interactCol = Color{ 255, 255, 255, 200 };
                        const Rectangle interactRec = { screenWidth / 2 - 20, screenHeight / 2 - 20, 40, 40 };
                        DrawRectangleLinesEx(interactRec, 10, interactCol);
                    }
                }

                //Backpack HUD
                {
                    const Color backpackCol = Color{ 200, 189, 0, 200 };
                    const Rectangle backpackRec = { 10, screenHeight - 210, 20, 200 };

                    

                    DrawText("backpack", 40, screenHeight - 80, 20, backpackCol);
                    DrawText((std::to_string(player.invetoryWeight) + " kg").c_str(), 40, screenHeight - 60, 50, backpackCol);
                    
                    DrawRectangleLinesEx(backpackRec, 5, backpackCol);
                    DrawRectangle(backpackRec.x,
                        backpackRec.y + (backpackRec.height * (1 - player.GetBackpackPercent())),
                        backpackRec.width, backpackRec.height * player.GetBackpackPercent(),
                        backpackCol);

                    
                }

                //HP HUD
                {
                    const Color hpCol = Color{ 200, 0, 0, 200 };
                    const Rectangle hpRec = { screenWidth - 30, screenHeight - 210, 20, 200 };

                    DrawText("health", screenWidth - 200, screenHeight - 80, 20, hpCol);
                    DrawText((std::to_string(player.hpPlayer) + " hp").c_str(), screenWidth - 200, screenHeight - 60, 50, hpCol);

                    DrawRectangleLinesEx(hpRec, 5, hpCol);
                    DrawRectangle(
                        hpRec.x,
                        hpRec.y + (hpRec.height * (1 - (float)player.hpPlayer / (float)100)),
                        hpRec.width,
                        hpRec.height * ((float)player.hpPlayer / (float)100),
                        hpCol);

                    if (player.hpPlayer <= 0) {
                        DrawRectangle(0, 0, screenWidth, screenHeight, Color{ 230, 41, 55, 100 });
                        DrawText("YOU ARE DIED!", screenWidth / 2 - 350, screenHeight / 2 - 30, 68, hpCol);
                        DrawText("Press R to restart game", screenWidth / 2 - 350, screenHeight / 2 + 30, 60, hpCol);
                    }
                }

                //DrawHomeLocation HUD
                {
                    if (!HQ->isQuotaComplete) {
                        vec3 homeLoc = HQ->GetPosition();

                        if (Vector3DotProduct(player.CameraRay().direction, Vector3Normalize(homeLoc - player.camera.position)) > 0) {
                            const Color homeCol = Color{ 0, 229, 0, 200 };
                            vec2 coord = GetWorldToScreen(homeLoc, player.camera);
                            Rectangle homeRec = { coord.x - 15, coord.y - 15, 30, 30 };
                            DrawRectangleLinesEx(homeRec, 5, homeCol);
                            std::string homeStr = "Home/HQ " + std::to_string((int)Vector3Distance(homeLoc, player.camera.position) / 5) + " m\ndeliver items here";
                            DrawText(homeStr.c_str(), homeRec.x - 15, homeRec.y + 40, 20, homeCol);
                        }
                    } else {
                        vec3 homeLoc = bed->GetPosition();

                        if (Vector3DotProduct(player.CameraRay().direction, Vector3Normalize(homeLoc - player.camera.position)) > 0) {
                            const Color homeCol = Color{ 0, 229, 0, 200 };
                            vec2 coord = GetWorldToScreen(homeLoc, player.camera);
                            Rectangle homeRec = { coord.x - 15, coord.y - 15, 30, 30 };
                            DrawRectangleLinesEx(homeRec, 5, homeCol);
                            std::string homeStr = "Bed " + std::to_string((int)Vector3Distance(homeLoc, player.camera.position) / 5) + " m\nend your day";
                            DrawText(homeStr.c_str(), homeRec.x - 15, homeRec.y + 40, 20, homeCol);
                        }
                    }

                }

                //quota HUD
                {
                    const Color quotaCol = Color{ 200, 59, 0, 200 };
                    DrawText("Remaining quota: ", screenWidth - 250, 20, 20, quotaCol);

                    DrawText(HQ->isQuotaComplete ? "Complete" : (std::to_string(HQ->quota - HQ->collectedQuota) + " kg").c_str(), screenWidth - 250, 40, 50, quotaCol);

                }

                //HP HUD
                {
                    const Color backpackCol = Color{ 200, 189, 0, 200 };
                    const Rectangle backpackRec = { 10, screenHeight - 210, 20, 200 };

                    DrawText("backpack", 40, screenHeight - 80, 20, backpackCol);
                    DrawText((std::to_string(player.invetoryWeight) + " kg").c_str(), 40, screenHeight - 60, 50, backpackCol);
                    DrawRectangleLinesEx(backpackRec, 5, backpackCol);
                    DrawRectangle(backpackRec.x,
                        backpackRec.y + (backpackRec.height * (1 - player.GetBackpackPercent())),
                        backpackRec.width, backpackRec.height * player.GetBackpackPercent(),
                        backpackCol);
                }

            }
        } //-HUD
        
    }
    EndDrawing();
    //----------------------------------------------------------------------------------
}

void DrawLoading() {
    BeginDrawing();
    DrawRectangle(0, 0, screenWidth, screenHeight, BLACK);
    DrawText("LOADING...", screenWidth / 2 - 120, screenHeight / 2 - 20, 40, WHITE);
    EndDrawing();
}

void GameRestart() {
    PlayerFP& player = PlayerFP::Instance();

    map->Generate(8);
    HQ = new HQ_InteractionPoint({ {90 + 11.9, 90 + 26.9, 2}, QuaternionIdentity(), Vector3Ones });
    bed = new bed_InteractionPoint(HQ, { {90 + 20.3, 90 + 32.1, 2}, QuaternionIdentity(), Vector3Ones });

    for (size_t i = 0; i < fmin(player.day,2); i++) {
        vec3 pos = Vector3RandomInVolume({ 300,300,20 });

        enemy = new EnemyTV();
        enemy->SetTranform({ pos, QuaternionFromEuler(PI / 2,0,0), { 1,1,1 } });
    }
    
}