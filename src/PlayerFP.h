#pragma once
#include "raylib.h"
#include "raymath.h"
#include "VVADExtras.h"
#include "Collision\CollisionManager.h"

// Movement constants
#define GRAVITY         32.0f
#define MAX_SPEED       25.0f
#define CROUCH_SPEED     4.0f
#define JUMP_FORCE      12.0f
#define MAX_ACCEL      120.0f
// Grounded drag
#define FRICTION         0.86f
// Increasing air drag, increases strafing speed
#define AIR_DRAG         0.86f
// Responsiveness for turning movement direction to looked direction
#define CONTROL         15.0f
#define CROUCH_HEIGHT    0.0f
#define STAND_HEIGHT     .6f
#define BOTTOM_HEIGHT    .2f

#define MAX_MOVEMENT_COLLISIONS 10

class PlayerFP
{
public:
    Camera camera;

    Vector3 position;
    Vector3 velocity;
    Vector3 dir;
    bool isGrounded;

    Vector2 lookRotation = { 0.f,0.f };
    Vector2 lean = { 0 };
    float headTimer = 0.0f;
    float walkLerp = 0.0f;
    float headLerp = STAND_HEIGHT;

    Vector2 sensitivity = { 0.001f, 0.001f };

    float playerSize = 0.3f;
    float floorAngle = 0.6f;

    PlayerFP();
    PlayerFP(Vector3 loc);
    void Init(Vector3 loc);

    void Update(float d, CollisionManager* cMngr);

    void UpdateCameraPos();
    void UpdateCameraFPS(Camera* camera);

    float GetCurrentPlayerHeight();

    Ray CameraRay();
};

