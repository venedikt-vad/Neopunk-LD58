#include "EnemyTV.h"
#include "PlayerFP.h"
#include <iostream>
#include <VVADExtras.h>

bool IsCameraLookingAtObject(Camera3D camera, Vector3 objectPosition, float threshold = 0.8f)
{
    Vector3 cameraForward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    Vector3 toObject = Vector3Normalize(Vector3Subtract(objectPosition, camera.position));
    float dot = Vector3DotProduct(cameraForward, toObject);

    return (dot >= threshold);
}

EnemyTV::EnemyTV()
{
    modelTV = LoadModel("resources/TV.obj");
    objectTransform = {{0.f, 0.f, 3.f}, QuaternionFromEuler(0, 0, 0), {1, 1, 1}};
    modelTV.transform = TransformToMatrix(objectTransform);
}

EnemyTV::~EnemyTV()
{
    UnloadModel(modelTV);
}

void EnemyTV::Update(float dt)
{
    PlayerFP &playerInstance = PlayerFP::Instance();
    CollisionManager& cMngrInsance = CollisionManager::Instance();

    Vector3 dir = Vector3Subtract(playerInstance.camera.position, objectTransform.translation);
    Vector3 dirNorm = Vector3Normalize(dir);

    Quaternion qForward = QuaternionFromForward(dir);  
    Quaternion qFix = QuaternionFromAxisAngle( { 0, 1, 0 }, PI/2);
    Quaternion q = QuaternionSlerp(objectTransform.rotation, QuaternionMultiply(qForward, qFix), 0.05);
    
    Vector3 direction = playerInstance.camera.position - objectTransform.translation;
    float distance = Vector3Length(direction);

    Ray r = { objectTransform.translation, Vector3Normalize(direction) };
    RayCollision collision = cMngrInsance.GetRayCollision(r,true);
    
    bool isLooking = IsCameraLookingAtObject(playerInstance.camera, objectTransform.translation);

    std::cout<< IsCameraLookingAtObject(playerInstance.camera, objectTransform.translation) << std::endl;

    SetTranform(
        {objectTransform.translation + dirNorm * !isLooking * 0.05,
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