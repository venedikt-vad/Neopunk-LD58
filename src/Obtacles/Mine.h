#pragma once

#include <map>
#include <string>
#include "Object.h"
#include "raylib.h"
#include "Lights\LightManager.h"
#include <Sound/Sound3d.h>

class Mine : public Object {
public:
    Mine();
    ~Mine();

    void DrawObject() override;
    void Update(float dt) override;
    void SetTranform(Transform transform) override;
private:
    Vector3 startPos;
    Vector3 endPos;

    Model model;
    Sound3d* expSound;
    LM_Light light;

    bool isDestroy = false;
};