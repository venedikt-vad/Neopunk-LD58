#pragma once

#include <map>
#include <string>
#include "Object.h"
#include "raylib.h"
#include <Sound/Sound3d.h>

class Laser : public Object {
public:
    Laser();
    ~Laser();

    void DrawObject() override;
    void Update(float dt) override;
    void SetTranform(Transform transform) override;
private:
    Vector3 startPos;
    Vector3 endPos;

    float lastTime = 0.f;

    Sound3d* sound;
};