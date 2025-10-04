#include "EnemyTV.h"
#include "PlayerFP.h"
#include <iostream>
#include <VVADExtras.h>

Matrix LookAtMatrix(Vector3 position, Vector3 target, Vector3 up)
{
    Vector3 zAxis = Vector3Normalize(Vector3Subtract(target, position));
    Vector3 xAxis = Vector3Normalize(Vector3CrossProduct(up, zAxis));
    Vector3 yAxis = Vector3CrossProduct(zAxis, xAxis);

    Matrix result = {
        xAxis.x, yAxis.x, zAxis.x, 0.0f,
        xAxis.y, yAxis.y, zAxis.y, 0.0f,
        xAxis.z, yAxis.z, zAxis.z, 0.0f,
        position.x, position.y, position.z, 1.0f
    };
    return result;
}

EnemyTV::EnemyTV() {
    modelTV = LoadModel("resources/TV.obj");
    modelTV.transform = TransformToMatrix({ { 5.f, 0.f, 2.f }, QuaternionFromEuler(0,0,0), { 1,1,1 } });
}

EnemyTV::~EnemyTV() {
    UnloadModel(modelTV);
}

void EnemyTV::Update(float dt) {
    PlayerFP &playerInstance = PlayerFP::Instance();

    // Vector3 enemyPos = { 0.f, 0.f, 2.f };
    // Vector3 target = playerInstance.position;
    // Vector3 up = {0.0f, 1.0f, 0.0f};

    // Matrix rotation = MatrixLookAt(enemyPos, target, up);
    // modelTV.transform = rotation;
    // // Matrix rotationOnly = MatrixRotateXYZ(...);

    // modelTV.transform.m30 = enemyPos.x;
    // modelTV.transform.m31 = enemyPos.y;
    // modelTV.transform.m32 = enemyPos.z;

    objectTransform.translation =  { 5.f, 0.f, 2.f };
    Quaternion q = QuaternionFromVector3ToVector3(objectTransform.translation, playerInstance.position);

    // Matrix enemyTransform = LookAtMatrix(objectTransform.translation, playerInstance.position, {0.0f, 1.0f, 0.0f});
    modelTV.transform = TransformToMatrix({ { 5.f, 0.f, 2.f }, q, { 1,1,1 } });;
}

void EnemyTV::DrawObject() {
    DrawModel(modelTV, { 5.f, 0.f, 2.f }, 1.f, WHITE);
}

void EnemyTV::SetTranform(Transform transform) {
    modelTV.transform = TransformToMatrix(transform);
    objectTransform = transform;
}