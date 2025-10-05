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
#include "HQ_InteractionPoint.h"
#include "bed_InteractionPoint.h"

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

HQ_InteractionPoint* HQ;
bed_InteractionPoint* bed;

Model modelTV;

EnemyTV* enemy;

Laser* laser;
Mine* mine;

Texture2D texture;

Texture2D noizeTexture;

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
    noizeTexture = LoadTexture("resources/noise.png");

    mat = LoadMaterialDefault();
    mat.shader = sh1;

    ObjectManager& objManager = ObjectManager::Instance();

    
    //interactiveObjectTest = new PickableObject(LoadModel("resources/UnitCube.obj"), { { 0.f, 0.f, 0.f }, QuaternionFromEuler(PI / 2,0,0), { 0.1,0.1,0.1 } }, KEY_E);

    modelTV = LoadModel("resources/TV.obj");
    modelTV.transform = TransformToMatrix({ { 0.f, 0.f, 0.f }, QuaternionFromEuler(0,0,0), { 1,1,1 } });

    enemy = new EnemyTV();
    enemy->SetTranform({ { 0.f, 0.f, 0.f }, QuaternionFromEuler(PI/2,0,0), { 1,1,1 } });

    laser = new Laser();
    laser->SetTranform({ { 1.f, 0.f, 1.f }, QuaternionFromEuler(PI/3,PI/3,PI/3), { 1,1,1 } });

    mine = new Mine();
    mine->SetTranform({ { 10.f, 0.f, 0.f }, QuaternionFromEuler(0, 0, 0), { 1,1,1 } });
    
    HQ = new HQ_InteractionPoint({ Vector3Zeros, QuaternionIdentity(), Vector3Ones});

    // Ambient light level (some basic lighting)
    int ambientLoc = GetShaderLocation(sh1, "ambient");
    float ambientValues[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
    SetShaderValue(sh1, ambientLoc, ambientValues, SHADER_UNIFORM_VEC4);
    
    gLightMgr = new LightManager(sh1);

    map = new MapGenerator(sh1, gLightMgr);

    CollisionManager& cMngr = CollisionManager::Instance(map);

    PlayerFP& player = PlayerFP::Instance();

    map->Generate(8);


    laser = new Laser();
    laser->SetTranform({ { 1.f, 0.f, 1.f }, QuaternionFromEuler(PI / 3,PI / 3,PI / 3), { 1,1,1 } });

    mine = new Mine();
    mine->SetTranform({ { 90, 120, .4f }, QuaternionFromEuler(0, 0, 0), { 1,1,1 } });

    HQ = new HQ_InteractionPoint({ {90 + 11.9, 90 + 26.9, 2}, QuaternionIdentity(), Vector3Ones });
    bed = new bed_InteractionPoint(HQ, { {90 + 20.3, 90 + 32.1, 2}, QuaternionIdentity(), Vector3Ones });

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

    if (player.hpPlayer <= 0 && IsKeyPressed(KEY_R)) {
        player.hpPlayer = 100;

        BeginDrawing();
        DrawRectangle(0, 0, screenWidth, screenHeight, BLACK);
        DrawText("LOADING...", screenWidth / 2 - 170, screenHeight / 2 + 40, 40, WHITE);
        EndDrawing();

        map->Generate(8);
        HQ = new HQ_InteractionPoint({ {90 + 11.9, 90 + 26.9, 2}, QuaternionIdentity(), Vector3Ones });
        bed = new bed_InteractionPoint(HQ, { {90 + 20.3, 90 + 32.1, 2}, QuaternionIdentity(), Vector3Ones });
    }
    
    //em1->Update(d);
    //em2->Update(d);

    enemy->Update(d);

    door1->Update(d);
    //lights[0].position = player->camera.position;
    //lights[0].target = player->CameraRay().direction;
    
    //LM_Light& edit = gLightMgr->Get(0);
    //edit.angle = fmodf((float)GetTime(), 1.0f) * 45.f;

    if(!freezeLightCooling)gLightMgr->SyncToGPU(player.camera);

    objManager.UpdateObjects(d);

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing(); {
        ClearBackground(BLACK);
        
        BeginMode3D(player.camera);
        BeginShaderMode(sh1); {
            DrawCube({ 90,90,90 }, 3000, 3000, 3, BLACK);
            map->Draw();
            //DrawMesh(modelMap.meshes[0], modelMap.materials[0], mapMatrix);
            // DrawModel(modelTV, { 0.f, 0.f, 3.f }, 1.f, WHITE);
            //em1->Draw(player.camera);

            //em2->Draw(player.camera);

            objManager.DrawObjects();

            enemy->DrawObject();

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
        //DrawText(Vec3ToString(player.position).c_str(), 10, 50, 30, RED);
        //DrawText((FloatToString(Vector3Length(player.velocity)) + " | " + Vec3ToString(player.velocity)).c_str(), 10, 80, 30, player.isGrounded ? YELLOW : SKYBLUE);

        //HUD
        {
            
                if (bed->dayEndscreen) {
                    //Day end screen

                    DrawRectangle(0, 0, screenWidth, screenHeight, BLACK);
                    if (GetTime() - bed->dayEndscreenBeginTime < 3) {
                        DrawText("day finished", screenWidth / 2 - 170, screenHeight / 2 - 20, 40, WHITE);
                    } else if (GetTime() - bed->dayEndscreenBeginTime < 6) {
                        DrawText("you hear the metal creaking\ncity continue to grow...", screenWidth / 2 - 220, screenHeight / 2 - 50, 40, WHITE);
                    } else {
                        DrawText("LOADING...", screenWidth / 2 - 170, screenHeight / 2 + 40, 40, WHITE);

                        map->Generate(8);
                        HQ = new HQ_InteractionPoint({ {90 + 11.9, 90 + 26.9, 2}, QuaternionIdentity(), Vector3Ones });
                        bed = new bed_InteractionPoint(HQ, { {90 + 20.3, 90 + 32.1, 2}, QuaternionIdentity(), Vector3Ones });


                    }
                    
                } else {
                    //Player noisze tex
                    {
                        int x = GetRandomValue(0, 1000);
                        unsigned char a = player.noiseAmount * 100;
                        DrawTexture(noizeTexture, x, 0, Color{ 255, 255, 255, a });
                        DrawTexture(noizeTexture, x-1920, 0, Color{ 255, 255, 255, a });
                    }

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

                        const Color hpCol = Color{ 200, 0, 0, 200 };
                        const Rectangle hpRec = { screenWidth - 30, screenHeight - 210, 20, 200 };

                        DrawText("backpack", 40, screenHeight - 80, 20, backpackCol);
                        DrawText((std::to_string(player.invetoryWeight) + " kg").c_str(), 40, screenHeight - 60, 50, backpackCol);
                        DrawText("health", screenWidth - 200, screenHeight - 80, 20, hpCol);
                        DrawText((std::to_string(player.hpPlayer) + " hp").c_str(), screenWidth - 200, screenHeight - 60, 50, hpCol);
                        DrawRectangleLinesEx(backpackRec, 5, backpackCol);
                        DrawRectangle(backpackRec.x,
                            backpackRec.y + (backpackRec.height * (1 - player.GetBackpackPercent())),
                            backpackRec.width, backpackRec.height * player.GetBackpackPercent(),
                            backpackCol);

                        DrawRectangleLinesEx(hpRec, 5, hpCol);
                        DrawRectangle(
                            hpRec.x,
                            hpRec.y + (hpRec.height * (1 - (float)player.hpPlayer/(float)100)),
                            hpRec.width,
                            hpRec.height * ((float)player.hpPlayer/(float)100),
                            hpCol);

                        if (player.hpPlayer <= 0) {
                            DrawText("YOU ARE DIED!", screenWidth/2 - 350, screenHeight/2 - 30, 68, hpCol);
                            DrawText("Press R to restart game", screenWidth/2 - 350, screenHeight/2 + 30, 60, hpCol);
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

            
        }
        
        

        //screenWidth
        //screenHeight
    }
    EndDrawing();
    //----------------------------------------------------------------------------------
}
