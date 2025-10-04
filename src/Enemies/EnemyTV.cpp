#include "EnemyTV.h"
#include "PlayerFP.h"
#include <iostream>
#include <VVADExtras.h>

EnemyTV::EnemyTV()
{
    modelTV = LoadModel("resources/TV.obj");
    modelTV.transform = TransformToMatrix({{0.f, 0.f, 0.f}, QuaternionFromEuler(0, 0, 0), {1, 1, 1}});
    objectTransform = {{0.f, 0.f, 0.f}, QuaternionFromEuler(0, 0, 0), {1, 1, 1}};
}

EnemyTV::~EnemyTV()
{
    UnloadModel(modelTV);
}

void EnemyTV::Update(float dt)
{
    PlayerFP &playerInstance = PlayerFP::Instance();

    Vector3 dir = Vector3Subtract(playerInstance.position, objectTransform.translation);
    Vector3 dirNorm = Vector3Normalize(dir);

    Vector3 fwd = Vector3Subtract(playerInstance.position, objectTransform.translation);
    Quaternion qForward = QuaternionFromForward(fwd);  
    Quaternion qFix = QuaternionFromAxisAngle( { 0, 1, 0 }, PI/2);
    Quaternion q = QuaternionSlerp(objectTransform.rotation, QuaternionMultiply(qForward, qFix), 0.05);

    SetTranform(
        {objectTransform.translation + dirNorm * 0.05,
         q,
         objectTransform.scale});
}

void EnemyTV::DrawObject()
{
    DrawModel(modelTV, Vector3Zero(), 1.f, WHITE);
}

void EnemyTV::SetTranform(Transform transform)
{
    modelTV.transform = TransformToMatrix({objectTransform.translation, objectTransform.rotation, objectTransform.scale});
    objectTransform = transform;
}