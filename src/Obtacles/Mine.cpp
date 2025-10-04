#include "Mine.h"
#include "PlayerFP.h"
#include <iostream>
#include <VVADExtras.h>

Mine::Mine() {
    model = LoadModel("resources/mine.obj");
    expSound = new Sound3d("resources/sounds/explotion.mp3", 1.f);
    // light.type = LM_SPOT; light.enabled = LM_SIMPLE; light.radius = 5.f; light.angle = 60.f; light.color = { 255, 0, 0, 255 }; light.intensity = 15;
}

Mine::~Mine() {
    UnloadModel(model);
    delete expSound;
}

void Mine::Update(float dt)
{
    PlayerFP& playerInstance = PlayerFP::Instance();

    float maxDist = 1.f;
    bool isPlayerCollision = Vector3Distance(playerInstance.position, objectTransform.translation) <= maxDist;
    
    if (isPlayerCollision && !isDestroy) {
        //TODO hit player;
        expSound->Play();
        isDestroy = true;
    }

    if (isPlayerCollision) {
        expSound->SetSoundPosition(playerInstance.camera, objectTransform.translation);
    }
}

void Mine::DrawObject()
{
    if (!isDestroy) {
        DrawModel(model, Vector3Zero(), 1.f, WHITE);
    }
}

void Mine::SetTranform(Transform transform)
{
    Quaternion rotX90 = QuaternionFromAxisAngle({1, 0, 0}, 90.0f * DEG2RAD);
    model.transform = TransformToMatrix({transform.translation, QuaternionMultiply(rotX90, transform.rotation), transform.scale});
    objectTransform = transform;

    std::cout << model.transform.m1
    << model.transform.m2 
    << model.transform.m3 
    << model.transform.m4 
    << model.transform.m5 
    << model.transform.m6 
    << model.transform.m7 
    << model.transform.m8 
    << model.transform.m9 
    << model.transform.m10 
    << model.transform.m11 
     << std::endl;

    // light.position = transform.translation;
    // light.direction = Vector3Scale(Vector3UnitZ, 1);
}