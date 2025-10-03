#include "PlayerFP.h"
#include <iostream>

#define NORMALIZE_INPUT

namespace VLV {

    PlayerFP::PlayerFP() {
        Init({ 0,0,0.029 });
    }

    PlayerFP::PlayerFP(Vector3 loc) {
        Init(loc);
    }

    PlayerFP& PlayerFP::Instance()
    {
        static PlayerFP pFP;
        return pFP;
    }

    PlayerFP& PlayerFP::Instance(Vector3 loc)
    {
        static PlayerFP pFP;
        return pFP;
    }

    void PlayerFP::Init(Vector3 loc) {
        position = loc;
        velocity = Vector3Zero();
        dir = Vector3Zero();

        camera.fovy = 60.0f;
        camera.projection = CAMERA_PERSPECTIVE;
        UpdateCameraPos();
        UpdateCameraFPS(&camera);

        DisableCursor();
    }

    void PlayerFP::Update(float d, CollisionManager* cMngr) {

        //Inputs
        Vector2 mouseDelta = GetMouseDelta();
        lookRotation.x -= mouseDelta.x * sensitivity.x;
        lookRotation.y += mouseDelta.y * sensitivity.y;

        int side = (IsKeyDown(KEY_D) - IsKeyDown(KEY_A));
        int forward = (IsKeyDown(KEY_W) - IsKeyDown(KEY_S));
        bool crouching = IsKeyDown(KEY_LEFT_CONTROL);
        bool jumpPressed = IsKeyPressed(KEY_SPACE);

        float playerHeight = GetCurrentPlayerHeight();
        vec3 posBeforeMovement = position;

        headLerp = Lerp(headLerp, (playerHeight), 20.0f * d);

        if (isGrounded && ((forward != 0) || (side != 0))) {
            headTimer += d * 3.0f;
            walkLerp = Lerp(walkLerp, 1.0f, 10.0f * d);
            camera.fovy = Lerp(camera.fovy, 55.0f, 5.0f * d);
        }
        else {
            walkLerp = Lerp(walkLerp, 0.0f, 10.0f * d);
            camera.fovy = Lerp(camera.fovy, 60.0f, 5.0f * d);
        }

        lean.x = Lerp(lean.x, side * 0.02f, 10.0f * d);
        lean.y = Lerp(lean.y, forward * 0.015f, 10.0f * d);

        Vector2 input = { (float)side, -(float)forward };

#if defined(NORMALIZE_INPUT)
        // Slow down diagonal movement
        if ((side != 0) && (forward != 0)) input = Vector2Normalize(input);
#endif


        Vector3 velocityXY = { velocity.x,velocity.y, 0 };
        Vector3 movementVector = velocityXY * d;
        movementVector.z = 0;

        float headOffset = playerHeight + PLAYER_RADIUS;

        int collisionCount = 0;
        do {
            Ray moveRayHead = { (position + (Vector3UnitZ * (headOffset + PLAYER_HEAD_SPACE))), Vector3Normalize(movementVector) };
            Ray moveRayCenter = { (position + Vector3UnitZ * (playerHeight / 2)), Vector3Normalize(movementVector) };
            //Ray moveRayBottom = { (position), Vector3Normalize(movementVector) };
            SphereTraceCollision collisionDataHead = cMngr->GetSphereCollision(moveRayHead, PLAYER_RADIUS);
            SphereTraceCollision collisionData = cMngr->GetSphereCollision(moveRayCenter, PLAYER_RADIUS);

            //bool foundCollision = false;
            /*if (collisionData.hit && collisionData.distance <= Vector3Length(movementVector)) {
                foundCollision = true;
            } else {
                collisionData.distance = Vector3Length(movementVector) * 10;
            }
            if (collisionDataHead.hit && collisionDataHead.distance <= Vector3Length(movementVector)) {
                foundCollision = true;
                head = true;
                if (collisionDataHead.distance < collisionData.distance)collisionData = collisionDataHead;
            }*/

            bool head = true;

            if (!(collisionDataHead.hit && collisionDataHead.distance <= Vector3Length(movementVector))) {
                if (!(collisionData.hit && collisionData.distance <= Vector3Length(movementVector))) {
                    position += movementVector;
                    break;
                }
                else {
                    collisionDataHead = collisionData;
                    head = false;
                }

            }

            vec3 newPos = collisionDataHead.point - (head ? (Vector3UnitZ * (Vector3UnitZ * (headOffset + PLAYER_HEAD_SPACE))) : (Vector3UnitZ * (playerHeight / 2)));
            movementVector -= newPos - position;
            position = newPos;

            // Project movement and velocity onto collision plane
            if (abs(Vector3DotProduct(collisionDataHead.normal, Vector3UnitZ)) >= FLOOR_ANGLE) {
                velocity = VectorPlaneProject(velocity, collisionDataHead.normal);
            }
            else {
                collisionDataHead.normal = Vector3Normalize({ collisionDataHead.normal.x, collisionDataHead.normal.y, 0 });
                velocityXY = VectorPlaneProject(velocityXY, collisionDataHead.normal);
            }
            movementVector = VectorPlaneProject(movementVector, collisionDataHead.normal);


            // If remaining movement is very small, ignore it
            if (Vector3Length(movementVector) < 0.001f) {
                break;
            }

            collisionCount++;
            if (collisionCount > MAX_MOVEMENT_COLLISIONS) {
                position = posBeforeMovement;
                break;
            }

        } while (Vector3Length(movementVector) > 0.001f);

        velocity.x = velocityXY.x;
        velocity.y = velocityXY.y;

        //Gravity and grav.collision
        float movementZ = velocity.z * d;
        vec3 gravRayDir = Vector3Normalize({ 0,0,(velocity.z == 0 ? -1 : velocity.z) });

        if (movementZ <= 0) {
            //Fall
            Ray gravRay = { (position + (Vector3UnitZ * (headOffset + PLAYER_HEAD_SPACE))), gravRayDir };
            SphereTraceCollision gravCollision = cMngr->GetSphereCollision(gravRay, PLAYER_RADIUS - 0.01);
            if ((gravCollision.hit && (gravCollision.distance <= (abs(movementZ) + headOffset + PLAYER_HEAD_SPACE + 0.03))) || gravCollision.initialHit) {
                position = gravCollision.point;

                if (Vector3DotProduct(gravCollision.normal, Vector3UnitZ) >= FLOOR_ANGLE) {
                    velocity.z = 0.0f;
                    isGrounded = true;
                }
                else {
                    isGrounded = false;
                    velocity = VectorPlaneProject(velocity, gravCollision.normal);
                }
            }
            else {
                isGrounded = false;
                position += Vector3UnitZ * movementZ;
            }

        }
        else {
            //Jump
            Ray gravRay = { (position + (Vector3UnitZ * .1f)), gravRayDir };
            SphereTraceCollision gravCollision = cMngr->GetSphereCollision(gravRay, PLAYER_RADIUS - 0.01);
            if ((gravCollision.hit && (gravCollision.distance <= (abs(movementZ) + headOffset + PLAYER_HEAD_SPACE))) || gravCollision.initialHit) {
                velocity = VectorPlaneProject(velocity, gravCollision.normal);
            }
            else {
                position += Vector3UnitZ * movementZ;
            }
        }

        velocity.z -= GRAVITY * d;

        //Inputs
        Vector3 front = { cos(lookRotation.x), sin(lookRotation.x), 0.f };
        Vector3 right = { sin(-lookRotation.x), cos(-lookRotation.x), 0.f };

        Vector3 desiredDir = (front * input.y) + (right * input.x);

        //Drag/friction
        velocity = velocity * (isGrounded ? FRICTION : AIR_DRAG);

        //Accel calc
        float accel = WALK_ACCEL * (isGrounded ? 1 : AIR_CONTROL) * d;
        vec3 velChange = desiredDir * accel;
        if (isGrounded) {
            velChange = Vector3ClampValue(velocity + velChange, 0, crouching ? CROUCH_SPEED : WALK_SPEED) - velocity;
        }
        velocity += velChange;

        if (isGrounded && jumpPressed) {
            velocity.z = JUMP_FORCE;
            isGrounded = false;
        }

        UpdateCameraPos();
        UpdateCameraFPS(&camera);
    }

    void PlayerFP::UpdateCameraPos() {
        camera.position = {
                position.x,
                position.y,
                position.z + (GetCurrentPlayerHeight() + headLerp),
        };
    }

    void PlayerFP::UpdateCameraFPS(Camera* camera) {
        const Vector3 up = { 0.0f, 0.0f, 1.0f };
        const Vector3 targetOffset = { -1.0f, 0.0f, 0.0f };

        // Left and right
        Vector3 yaw = Vector3RotateByAxisAngle(targetOffset, up, lookRotation.x);

        // Clamp view up
        float maxAngleUp = Vector3Angle(up, yaw);
        maxAngleUp -= 0.001f; // Avoid numerical errors
        if (-(lookRotation.y) > maxAngleUp) { lookRotation.y = -maxAngleUp; }

        // Clamp view down
        float maxAngleDown = Vector3Angle(Vector3Negate(up), yaw);
        maxAngleDown *= -1.0f; // Downwards angle is negative
        maxAngleDown += 0.001f; // Avoid numerical errors
        if (-(lookRotation.y) < maxAngleDown) { lookRotation.y = -maxAngleDown; }

        // Up and down
        Vector3 right = Vector3Normalize(Vector3CrossProduct(yaw, up));

        // Rotate view vector around right axis
        float pitchAngle = -lookRotation.y - lean.y;
        pitchAngle = Clamp(pitchAngle, -PI / 2 + 0.0001f, PI / 2 - 0.0001f); // Clamp angle so it doesn't go past straight up or straight down
        Vector3 pitch = Vector3RotateByAxisAngle(yaw, right, pitchAngle);

        // Head animation
        // Rotate up direction around forward axis
        float headSin = (float)sin(headTimer * PI);
        float headCos = (float)cos(headTimer * PI);
        const float stepRotation = 0.01f;
        camera->up = Vector3RotateByAxisAngle(up, pitch, headSin * stepRotation + lean.x);

        // Camera BOB
        const float bobSide = 0.1f;
        const float bobUp = 0.15f;
        Vector3 bobbing = Vector3Scale(right, headSin * bobSide);
        bobbing.y = fabsf(headCos * bobUp);

        camera->position = Vector3Add(camera->position, Vector3Scale(bobbing, walkLerp));
        camera->target = Vector3Add(camera->position, pitch);
    }

    float PlayerFP::GetCurrentPlayerHeight() {
        return IsKeyDown(KEY_LEFT_CONTROL) ? CROUCH_HEIGHT : STAND_HEIGHT;
    }

    Ray PlayerFP::CameraRay() {
        return GetCameraRay(camera);
    }
}