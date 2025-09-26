#include "PlayerFP.h"

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

void PlayerFP::Update(float d, Model modelMap, Matrix mapMatrix) {

    //Inputs
    Vector2 mouseDelta = GetMouseDelta();
    lookRotation.x -= mouseDelta.x * sensitivity.x;
    lookRotation.y += mouseDelta.y * sensitivity.y;

    int side = (IsKeyDown(KEY_D) - IsKeyDown(KEY_A));
    int forward = (IsKeyDown(KEY_W) - IsKeyDown(KEY_S));
    bool crouching = IsKeyDown(KEY_LEFT_CONTROL);
    bool jumpPressed = IsKeyPressed(KEY_SPACE);

    headLerp = Lerp(headLerp, (crouching ? CROUCH_HEIGHT : STAND_HEIGHT), 20.0f * d);

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

    Vector2 input = { side, -forward };

#if defined(NORMALIZE_INPUT)
    // Slow down diagonal movement
    if ((side != 0) && (forward != 0)) input = Vector2Normalize(input);
#endif

    Ray gravRay = { (position + Vector3UnitZ * playerSize), Vector3UnitZ * -1 };
    Ray gravRay2 = { (position + Vector3UnitZ * playerSize + Vector3Normalize(VectorPlaneProject(velocity,Vector3UnitZ))* playerSize ), Vector3UnitZ*-1 };
    RayCollision collisionDataFall = GetRayCollisionMesh(gravRay, modelMap.meshes[0], mapMatrix);
    RayCollision collisionDataFall2 = GetRayCollisionMesh(gravRay2, modelMap.meshes[0], mapMatrix);
    if ((collisionDataFall.hit && collisionDataFall.distance <= playerSize + 0.0001)||(collisionDataFall2.hit && collisionDataFall2.distance <= playerSize + 0.001)) {
        if (Vector3DotProduct(collisionDataFall.normal, Vector3UnitZ)>=floorAngle) {
            velocity.z = 0.0f;
            isGrounded = true;
            position = collisionDataFall.point;
        } else {
            isGrounded = false;
            velocity = VectorPlaneProject(velocity, collisionDataFall.normal);
        }
       
    } else {
        isGrounded = false;
    }
     
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

    Vector3 rightV = Vector3CrossProduct(Vector3Normalize(movementVector), Vector3UnitZ);

    int collisionCount = 0;

    do {
        // Using 3 Rays (from center and sides) to check for collisions
        Ray moveRay = { (position + Vector3UnitZ * playerSize), Vector3Normalize(movementVector) };
        RayCollision collisionData = GetRayCollisionMesh(moveRay, modelMap.meshes[0], mapMatrix);

        // Check if any ray has valid collision and distance is within movement range
        bool foundCollision = collisionData.hit && collisionData.distance - playerSize <= Vector3Length(movementVector);
       
        // Find the closest collision
        RayCollision closestCollision = collisionData;

        if (!foundCollision) {
            position += movementVector;
            break;
        }

        // Set new position
        vec3 newPos = closestCollision.point + closestCollision.normal * playerSize - Vector3UnitZ * playerSize;
        movementVector -= newPos - position;
        position = newPos;

        // Project movement vector onto collision plane to slide
        movementVector = VectorPlaneProject(movementVector, closestCollision.normal);
        
        //Right now bugged but should work
        //velocity = VectorPlaneProject(velocity, closestCollision.normal);

        // If remaining movement is very small, stop sliding
        if (Vector3Length(movementVector) < 0.001f) {
            break;
        }

        // Increment collision count
        collisionCount++;

    } while (collisionCount < 6 && Vector3Length(movementVector) > 0.001f);


    Ray moveRayL = { (position + Vector3UnitZ * playerSize - rightV * playerSize), Vector3Normalize(movementVector) };
    Ray moveRayR = { (position + Vector3UnitZ * playerSize + rightV * playerSize), Vector3Normalize(movementVector) };
    RayCollision collisionDataL = GetRayCollisionMesh(moveRayL, modelMap.meshes[0], mapMatrix);
    RayCollision collisionDataR = GetRayCollisionMesh(moveRayR, modelMap.meshes[0], mapMatrix);

    if (collisionDataL.hit) {
        if (Vector3DotProduct(collisionDataL.normal, Vector3UnitZ) < floorAngle) {
            vec3 posCorrection = PointPlaneProject(position, collisionDataL.point, collisionDataL.normal);
            if (Vector3Distance(posCorrection, position) < playerSize) {
                position = posCorrection + collisionDataL.normal * playerSize;
            }
        }
        
    }
    if (collisionDataR.hit) {
        if (Vector3DotProduct(collisionDataR.normal, Vector3UnitZ) < floorAngle) {
            vec3 posCorrection = PointPlaneProject(position, collisionDataR.point, collisionDataR.normal);
            if (Vector3Distance(posCorrection, position) < playerSize) {
                position = posCorrection + collisionDataR.normal * playerSize;
            }
        }
    }


    // Fancy collision system against the floor
    if (position.z <= -10.0f) {
        position.z = -10.0f;
        velocity.z = 0.0f;
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
    float headSin = sin(headTimer * PI);
    float headCos = cos(headTimer * PI);
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

Ray PlayerFP::CameraRay() {
    return GetCameraRay(camera);
}
