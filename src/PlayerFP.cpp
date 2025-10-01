#include "PlayerFP.h"
#include <iostream>

//#define NORMALIZE_INPUT

PlayerFP::PlayerFP() {
    Init(Vector3Zero());
}

PlayerFP::PlayerFP(Vector3 loc) {
    Init(loc);
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
    } else {
        walkLerp = Lerp(walkLerp, 0.0f, 10.0f * d);
        camera.fovy = Lerp(camera.fovy, 60.0f, 5.0f * d);
    }

    lean.x = Lerp(lean.x, side * 0.02f, 10.0f * d);
    lean.y = Lerp(lean.y, forward * 0.015f, 10.0f * d);


    //

    Vector2 input = { (float)side, -forward };

#if defined(NORMALIZE_INPUT)
    // Slow down diagonal movement
    if ((side != 0) && (forward != 0)) input = Vector2Normalize(input);
#endif

     
    if (!isGrounded) velocity.z -= GRAVITY * d;

    if (isGrounded && jumpPressed) {
        velocity.z = JUMP_FORCE;
        isGrounded = false;
    }

    Vector3 front = { cos(lookRotation.x), sin(lookRotation.x), 0.f  };
    Vector3 right = { sin(-lookRotation.x), cos(-lookRotation.x), 0.f };

    Vector3 desiredDir = (front * input.y) + (right * input.x);
    dir = Vector3Lerp(dir, desiredDir, CONTROL * d);

    float decel = (isGrounded ? FRICTION : AIR_DRAG);
    Vector3 hvel = { velocity.x * decel, velocity.y * decel, 0.0f  };

    float hvelLength = Vector3Length(hvel); // Magnitude
    if (hvelLength < (MAX_SPEED * 0.01f)) hvel = Vector3Zero();

    // This is what creates strafing
    float speed = Vector3DotProduct(hvel, dir);

    // Whenever the amount of acceleration to add is clamped by the maximum acceleration constant,
    // a Player can make the speed faster by bringing the direction closer to horizontal velocity angle
    float maxSpeed = (crouching ? CROUCH_SPEED : MAX_SPEED);
    float accel = Clamp(maxSpeed - speed, 0.f, MAX_ACCEL * d);
    hvel.x += dir.x * accel;
    hvel.y += dir.y * accel;

    velocity.x = hvel.x;
    velocity.y = hvel.y;

    Vector3 movementVector = velocity * d;

    float headOffset = playerHeight + playerSize;
    int collisionCount = 0;
    do {
        Ray moveRayHead = { (position + (Vector3UnitZ * headOffset)), Vector3Normalize(movementVector) };
        Ray moveRayCenter = { (position + Vector3UnitZ * (playerHeight/2)), Vector3Normalize(movementVector) };
        Ray moveRayBottom = { (position), Vector3Normalize(movementVector) };
        SphereTraceCollision collisionDataHead = cMngr->GetSphereCollision(moveRayHead, playerSize);
        SphereTraceCollision collisionData = cMngr->GetSphereCollision(moveRayBottom, playerSize);

        bool head = false;
        bool foundCollision = false;
        if (collisionData.hit && collisionData.distance <= Vector3Length(movementVector)) {
            foundCollision = true;
        } else {
            collisionData.distance = Vector3Length(movementVector) * 10;
        }
        if (collisionDataHead.hit && collisionDataHead.distance <= Vector3Length(movementVector)) {
            foundCollision = true;
            head = true;
            if (collisionDataHead.distance < collisionData.distance)collisionData = collisionDataHead;
        }

        if (!foundCollision) {
            position += movementVector;
            break;
        }

        vec3 newPos = head? (collisionData.point - (Vector3UnitZ * headOffset)) : (collisionData.point - (Vector3UnitZ * (playerHeight / 2)));
        movementVector -= newPos - position;
        position = newPos;

        if (abs(Vector3DotProduct(collisionData.normal, Vector3UnitZ)) >= floorAngle) {
        } else {
            collisionData.normal = Vector3Normalize({ collisionData.normal.x, collisionData.normal.y, 0 });
        }
        // Project movement and velocity onto collision plane
        movementVector = VectorPlaneProject(movementVector, collisionData.normal);
        velocity = VectorPlaneProject(velocity, collisionData.normal);
        

        // If remaining movement is very small, ignore it
        if (Vector3Length(movementVector) < 0.001f) {
            break;
        }

        collisionCount++;

    } while (collisionCount < MAX_MOVEMENT_COLLISIONS && Vector3Length(movementVector) > 0.001f);

    //std::cout << collisionCount << std::endl;

    //Prevent movement if too many collisions hit
    if (collisionCount >= MAX_MOVEMENT_COLLISIONS) {
        velocity = Vector3Zeros;
        position = posBeforeMovement;
        TraceLog(LOG_WARNING, "MOVEMENT FAILED: Too many collisions");
    }

    // Fancy collision system against the floor
    if (position.z <= -10.0f) {
        position.z = -10.0f;
        velocity.z = 0.0f;

    }

    //Additional floor check
    Ray gravRay = { (position + Vector3UnitZ * playerHeight), Vector3UnitZ * -1 };
    SphereTraceCollision gravCollision = cMngr->GetSphereCollision(gravRay, playerSize);
    if (gravCollision.hit && (gravCollision.distance <= playerHeight + 0.001)) {
        position = gravCollision.point;
        if (Vector3DotProduct(gravCollision.normal, Vector3UnitZ) >= floorAngle) {
            velocity.z = 0.0f;
            isGrounded = true;
        } else {
            isGrounded = false;
            //velocity = VectorPlaneProject(velocity, gravCollision.normal);
        }
    } else {
        isGrounded = false;
    }
    
    UpdateCameraPos();
    UpdateCameraFPS(&camera);
}

void PlayerFP::UpdateCameraPos() {
    camera.position = {
            position.x,
            position.y,
            position.z + (BOTTOM_HEIGHT + headLerp),
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
