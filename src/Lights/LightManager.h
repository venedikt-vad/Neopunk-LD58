#pragma once
#include "raylib.h"
#include <vector>
#include <algorithm>

#ifndef MAX_LIGHTS
#define MAX_LIGHTS 32
#endif

enum LM_LightType { LM_DIR = 0, LM_POINT = 1, LM_SPOT = 2 };
enum LM_Enabled   { LM_OFF = 0, LM_SIMPLE = 1, LM_SIMPLE_AND_VOLUMETRIC = 2 };

struct LM_Light {
    LM_LightType type = LM_POINT;
    LM_Enabled   enabled = LM_SIMPLE;
    Vector3      position{};
    Vector3      direction{};
    Color        color{255,255,255,255};
    float        radius = 5.0f;
    float        angle  = 45.0f;    // half-angle for spot cone in degrees
    float        intensity = 1.0f;  // NEW: scalar intensity multiplier
};

class LightManager {
public:
    explicit LightManager(Shader shader) : sh(shader) { cacheSlotLocations(); }

    // Authoring
    int  Add(const LM_Light& l) { lights.push_back(l); return (int)lights.size()-1; }
    void Clear() { lights.clear(); }
    LM_Light& Get(int id) { return lights[id]; }

    // Per-frame: choose best lights and upload to GPU
    void SyncToGPU(const Camera3D& cam);

    // Projection params (used by frustum tests)
    void SetCameraProjection(float nearZ, float farZ) { camNear = nearZ; camFar = farZ; }

    // Volumetric cap: maximum number of lights that render volumetrics this frame
    void SetVolumetricCap(int n) { volumetricCap = (n < 0) ? 0 : n; }
    int  GetVolumetricCap() const { return volumetricCap; }

    // Convenience
    void Enable(int id, LM_Enabled mode) { lights[id].enabled = mode; }
    void Disable(int id) { lights[id].enabled = LM_OFF; }

    int Count() { return lights.size(); }

private:
    struct SlotLocs {
        int enabled=-1, type=-1, position=-1, direction=-1, color=-1, radius=-1, angle=-1, intensity=-1;
    };

    Shader sh;
    SlotLocs slots[MAX_LIGHTS]{};
    std::vector<LM_Light> lights;

    // Cached uniform locations for each slot
    void cacheSlotLocations();

    // Scoring & culling
    static float scoreLight(const LM_Light& l, const Camera3D& cam, float aspect, float nearZ, float farZ);
    static bool  spotVisible(const LM_Light& l, const Camera3D& cam, float aspect, float nearZ, float farZ);
    static float rad(float deg){ return deg*0.01745329251f; }

    // GPU write helpers
    void writeSlot(int slot, const LM_Light& l, LM_Enabled effectiveMode);
    void disableSlot(int slot);

    // Camera projection info
    float camNear = 0.1f, camFar = 100.0f;

    // Volumetric lights cap per frame (0 = disable volumetrics globally)
    int volumetricCap = 16;
};
