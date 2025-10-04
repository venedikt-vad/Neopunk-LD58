#pragma once

#include <map>
#include <string>
#include "Object.h"
#include "raylib.h"
#include <Sound/Sound3d.h>

class EnemyTV : public Object {
public:
    EnemyTV();
    ~EnemyTV();

    void DrawObject() override;
    void Update(float dt) override;
    void SetTranform(Transform transform) override;
private:
    Model modelTV;
    Sound3d* voidSound;
    Sound3d* voicesSound;
};