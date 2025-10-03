#pragma once
#include "raylib.h"
#include "raymath.h"
#include "VVADExtras.h"
#include "Collision\CollisionManager.h"
#include "Sound/MultiInstrument.h"

// Movement constants
#define GRAVITY         32.0f

#define WALK_SPEED      25.0f
#define CROUCH_SPEED    4.0f

#define JUMP_FORCE     12.0f
//#define MAX_ACCEL  120.0f
#define WALK_ACCEL     150.f

// Grounded drag
#define FRICTION       0.86f //floor friction
// Increasing air drag, increases strafing speed
#define AIR_DRAG       0.99f //air friction
#define AIR_CONTROL    0.1f //amount of input accel applying when not grounded

// Responsiveness for turning movement direction to looked direction
//#define CONTROL         15.0f

#define PLAYER_RADIUS     .3f //radius of player capsule (adds to bbottom and top height too)
#define CROUCH_HEIGHT    0.0f 
#define STAND_HEIGHT      .6f

#define PLAYER_HEAD_SPACE .3f

#define MAX_MOVEMENT_COLLISIONS 10
#define FLOOR_ANGLE 0.8f //normal dot product with Z-axis


#define DEBUG_PLAYER
namespace VLV {

    class PlayerFP {
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

        //float playerSize = 0.3f;
        //float floorAngle = 0.8f;

        static PlayerFP& Instance();
        static PlayerFP& Instance(Vector3 loc);

        void Init(Vector3 loc);

        void Update(float d);

        void UpdateCameraPos();
        void UpdateCameraFPS(Camera* camera);

        float GetCurrentPlayerHeight();

        Ray CameraRay();
    private:
        PlayerFP();
        PlayerFP(Vector3 loc);

        ~PlayerFP() {};

        PlayerFP(PlayerFP const&) = delete;

        PlayerFP& operator= (PlayerFP const&) = delete;

        MultiInstrument* runSound;
    };
}
