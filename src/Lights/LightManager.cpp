#include "LightManager.h"
#include "raymath.h"
#include <cmath>
#include <algorithm>

#ifndef LM_PI
#define LM_PI 3.14159265358979323846f
#endif

static inline float dot3(Vector3 a, Vector3 b){ return a.x*b.x + a.y*b.y + a.z*b.z; }
static inline Vector3 sub3(Vector3 a, Vector3 b){ return {a.x-b.x, a.y-b.y, a.z-b.z}; }
static inline float len3(Vector3 v){ return sqrtf(dot3(v,v)); }
static inline Vector3 norm3(Vector3 v){ float l=len3(v); return (l>0)?Vector3Scale(v,1.0f/l):Vector3Zero(); }
static inline float deg2rad(float deg){ return deg * 0.01745329251f; }

// Helper: sphere vs camera frustum test
static bool SphereFrustumIntersect(const Vector3& sphereCenter, float R,
                                   const Camera3D& cam, float aspect,
                                   float nearZ, float farZ)
{
    const Vector3 fwd = norm3(sub3(cam.target, cam.position));
    const Vector3 right = norm3(Vector3CrossProduct(fwd, cam.up));
    const Vector3 upn = norm3(Vector3CrossProduct(right, fwd));

    const float halfFovY = deg2rad(cam.fovy * 0.5f);
    const float halfFovX = atanf(tanf(halfFovY) * aspect);

    const Vector3 v = sub3(sphereCenter, cam.position);
    const float d = len3(v);
    if (d <= 1e-6f) return true; // camera inside
    const float z = dot3(v, fwd);
    if (z < (nearZ - R)) return false;
    if (z > (farZ  + R)) return false;

    const float slack = (R >= d) ? (LM_PI*0.5f) : asinf(R/d);
    const float xAng = atan2f(dot3(v, right), z);
    const float yAng = atan2f(dot3(v, upn),   z);

    if (fabsf(xAng) > (halfFovX + slack)) return false;
    if (fabsf(yAng) > (halfFovY + slack)) return false;
    return true;
}

void LightManager::cacheSlotLocations() {
    for (int i=0;i<MAX_LIGHTS;i++) {
        slots[i].enabled   = GetShaderLocation(sh, TextFormat("lights[%i].enabled",   i));
        slots[i].type      = GetShaderLocation(sh, TextFormat("lights[%i].type",      i));
        slots[i].position  = GetShaderLocation(sh, TextFormat("lights[%i].position",  i));
        slots[i].direction = GetShaderLocation(sh, TextFormat("lights[%i].direction", i));
        slots[i].color     = GetShaderLocation(sh, TextFormat("lights[%i].color",     i));
        slots[i].radius    = GetShaderLocation(sh, TextFormat("lights[%i].radius",    i));
        slots[i].angle     = GetShaderLocation(sh, TextFormat("lights[%i].spotAngle", i));
        slots[i].intensity = GetShaderLocation(sh, TextFormat("lights[%i].intensity", i));
    }
}

void LightManager::disableSlot(int slot) {
    int off = (int)LM_OFF;
    SetShaderValue(sh, slots[slot].enabled, &off, SHADER_UNIFORM_INT);
}

void LightManager::writeSlot(int slot, const LM_Light& l, LM_Enabled effectiveMode) {
    int enabled = (int)effectiveMode;
    int type    = (int)l.type;
    float pos[3] = { l.position.x, l.position.y, l.position.z };
    float dir[3] = { l.direction.x, l.direction.y, l.direction.z };
    float col[4] = { l.color.r/255.f, l.color.g/255.f, l.color.b/255.f, l.color.a/255.f };

    SetShaderValue(sh, slots[slot].enabled,   &enabled, SHADER_UNIFORM_INT);
    SetShaderValue(sh, slots[slot].type,      &type,    SHADER_UNIFORM_INT);
    SetShaderValue(sh, slots[slot].position,  pos,      SHADER_UNIFORM_VEC3);
    SetShaderValue(sh, slots[slot].direction, dir,      SHADER_UNIFORM_VEC3);
    SetShaderValue(sh, slots[slot].color,     col,      SHADER_UNIFORM_VEC4);
    SetShaderValue(sh, slots[slot].radius,    &l.radius,SHADER_UNIFORM_FLOAT);
    SetShaderValue(sh, slots[slot].angle,     &l.angle, SHADER_UNIFORM_FLOAT);
    SetShaderValue(sh, slots[slot].intensity, &l.intensity, SHADER_UNIFORM_FLOAT);
}

// SPOT visibility using union of minimal cone sphere and base sphere, plus near-cone guard
bool LightManager::spotVisible(const LM_Light& l, const Camera3D& cam, float aspect, float nearZ, float farZ) {
    const Vector3 apex = l.position;
    const float h = fmaxf(l.radius, 0.0001f);
    const float theta = deg2rad(fmaxf(1.0f, l.angle)); // half-angle in radians
    const float r = h * tanf(theta);
    const Vector3 axis = norm3(l.direction);

    // Minimal enclosing sphere of cone
    const float R1 = (h*h + r*r) / (2.0f*h);
    const float a  = (h*h - r*r) / (2.0f*h);
    const Vector3 C1 = { apex.x + axis.x * a, apex.y + axis.y * a, apex.z + axis.z * a };

    // Base sphere
    const float R2 = r;
    const Vector3 C2 = { apex.x + axis.x * h, apex.y + axis.y * h, apex.z + axis.z * h };

    if (SphereFrustumIntersect(C1, R1, cam, aspect, nearZ, farZ)) return true;
    if (SphereFrustumIntersect(C2, R2, cam, aspect, nearZ, farZ)) return true;

    // Extra guard: if camera is near/inside cone segment, keep
    const Vector3 camToApex = sub3(cam.position, apex);
    float t = dot3(camToApex, axis); // signed distance along +axis from apex to camera
    t = fminf(fmaxf(t, 0.0f), h);
    const Vector3 closest = { apex.x + axis.x * t, apex.y + axis.y * t, apex.z + axis.z * t };
    const float radial = len3(sub3(cam.position, closest));
    const float localRadius = t * tanf(theta);
    if (radial <= localRadius + 0.35f) { // epsilon
        return true;
    }

    return false;
}

// Comparison for partial_sort
static bool ScoreGreater(const std::pair<int,float>& a, const std::pair<int,float>& b){
    return a.second > b.second;
}

float LightManager::scoreLight(const LM_Light& l, const Camera3D& cam, float aspect, float nearZ, float farZ) {
    if (l.enabled == LM_OFF) return -1e9f;

    // Visibility gates
    if (l.type == LM_SPOT) {
        if (!spotVisible(l, cam, aspect, nearZ, farZ)) return -1e9f;
    } else if (l.type == LM_POINT) {
        const float R = fmaxf(l.radius, 0.001f);
        if (!SphereFrustumIntersect(l.position, R, cam, aspect, nearZ, farZ)) return -1e9f;
    }
    // Directional lights are left always "visible".

    // Distance bias
    const Vector3 toL = sub3(l.position, cam.position);
    const float d = len3(toL);
    const float distBias = 1.0f / (0.5f + d);

    // Preference weights
    const float typeBonus = (l.type == LM_SPOT ? 0.25f : (l.type == LM_DIR ? 0.10f : 0.0f));
    const float rangeBias = fminf(l.radius, 20.0f) * 0.03f;

    return distBias*2.5f + rangeBias + typeBonus;
}

void LightManager::SyncToGPU(const Camera3D& cam) {
    const float aspect = (float)GetScreenWidth() / (float)GetScreenHeight();

    std::vector< std::pair<int,float> > picks;
    picks.reserve(lights.size());

    for (int i=0;i<(int)lights.size();++i) {
        const float sc = scoreLight(lights[i], cam, aspect, camNear, camFar);
        picks.push_back(std::make_pair(i, sc));
    }

    const int keep = (int)std::min<size_t>(MAX_LIGHTS, picks.size());
    if (keep > 0) {
        std::partial_sort(picks.begin(), picks.begin()+keep, picks.end(), ScoreGreater);
    }

    // Disable all slots first
    for (int s=0; s<MAX_LIGHTS; ++s) disableSlot(s);

    // Fill winners with volumetric cap enforcement
    int usedVolumetric = 0;
    for (int s=0; s<keep; ++s) {
        if (picks[s].second <= -1e8f) break; // none visible/enabled
        const LM_Light& L = lights[picks[s].first];
        LM_Enabled effective = L.enabled;
        if (effective == LM_SIMPLE_AND_VOLUMETRIC) {
            if (usedVolumetric >= volumetricCap) {
                effective = LM_SIMPLE; // downgrade gracefully
            } else {
                usedVolumetric++;
            }
        }
        writeSlot(s, L, effective);
    }
}
