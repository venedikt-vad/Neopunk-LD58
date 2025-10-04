#include "Laser.h"
#include "PlayerFP.h"
#include <iostream>
#include <VVADExtras.h>

Laser::Laser() {
    sound = new Sound3d("resources/sounds/laser.mp3", 0.5f, 1.f);
}

Laser::~Laser() {
    delete sound;
}

void Laser::Update(float dt)
{
    CollisionManager& cMngrInstance = CollisionManager::Instance();
    PlayerFP& playerInstance = PlayerFP::Instance();
    
    Vector3 dir = GetForwardVector(objectTransform);
    Ray r = { objectTransform.translation, dir };
    RayCollision collision = cMngrInstance.GetRayCollision(r,true);
    startPos = objectTransform.translation;
    endPos = collision.hit ? collision.point : dir * 1000.f;

    Vector3 playerPos = playerInstance.camera.position;

    float dist = 1.f;

    sound->SetSoundPosition(playerInstance.camera, ClosestPointOnLine(startPos, endPos, playerInstance.camera.position));
    if (!sound->IsPlayingSound()) {
        sound->Play();
    }

    bool isPlayerCollision = playerPos.x > (startPos.x - dist) && playerPos.x < (endPos.x + dist) &&
                             playerPos.y > (startPos.y - dist) && playerPos.y < (endPos.y + dist) &&
                             playerPos.z > (startPos.z - dist) && playerPos.z < (endPos.z + dist);
    //TODO hit player;
}

void Laser::DrawObject()
{
    DrawLine3D(startPos, endPos, GREEN);
}

void Laser::SetTranform(Transform transform)
{
    objectTransform = transform;
}