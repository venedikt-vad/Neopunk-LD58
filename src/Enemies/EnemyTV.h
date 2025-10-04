#pragma once

#include <map>
#include <string>
#include "Object.h"
#include "raylib.h"

class EnemyTV : public Object {
public:
    EnemyTV();
    ~EnemyTV();

    void DrawObject() override;
    void Update(float dt) override;
    void SetTranform(Transform transform) override;
private:
    Model modelTV;
};