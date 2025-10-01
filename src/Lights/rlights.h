/**********************************************************************************************
*
*   raylib.lights - Some useful functions to deal with lights data
*
*   CONFIGURATION:
*
*   #define RLIGHTS_IMPLEMENTATION
*       Generates the implementation of the library into the included file.
*       If not defined, the library is in header only mode and can be included in other headers 
*       or source files without problems. But only ONE file should hold the implementation.
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2017-2024 Victor Fisac (@victorfisac) and Ramon Santamaria (@raysan5)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#ifndef RLIGHTS_H
#define RLIGHTS_H

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define MAX_LIGHTS  8         // Max dynamic lights supported by shader

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

#include "raylib.h"
#include "raymath.h"


// Light data
typedef struct {   
    int type;
    int enabled;
    Vector3 position;
    Vector3 direction;
    Color color;
    float radius;
    float angle;
    
    // Shader locations
    int typeLoc;
    int enabledLoc;
    int positionLoc;
    int directionLoc;
    int colorLoc;
    int radiusLoc;
    int angleLoc;
} Light;

// Light type
typedef enum {
    LIGHT_DIRECTIONAL = 0,
    LIGHT_POINT = 1,
    LIGHT_SPOT = 2,
} LightType;

// Light type
typedef enum {
    LIGHT_DISABLED = 0,
    LIGHT_SIMPLE = 1,
    LIGHT_SIMPLE_AND_VOLUMETRIC = 2,
} LightEnabled;

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
Light CreateLight(int type, Vector3 position, Vector3 target, float radius, Color color, Shader shader, int enabled = 1);   // Create a light and get shader locations
void UpdateLightValues(Shader shader, Light light);         // Send light properties to shader
void UpdateLightsArray(Shader shader, Light lights[MAX_LIGHTS], Camera cam);         // Cull unseen lights and sort by distance

#ifdef __cplusplus
}
#endif

#endif // RLIGHTS_H


/***********************************************************************************
*
*   RLIGHTS IMPLEMENTATION
*
************************************************************************************/

#if defined(RLIGHTS_IMPLEMENTATION)

#include "raylib.h"

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
static int lightsCount = 0;    // Current amount of created lights

//----------------------------------------------------------------------------------
// Module specific Functions Declaration
//----------------------------------------------------------------------------------
// ...

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------

// Create a light and get shader locations
Light CreateLight(int type, Vector3 position, Vector3 target, float radius, Color color, Shader shader, int enabled)
{
    Light light = { 0 };

    if (lightsCount < MAX_LIGHTS)
    {
        light.enabled = enabled;
        light.type = type;
        light.position = position;
        light.direction = target;
        light.color = color;
        light.radius = radius;
        light.angle = 45;

        // NOTE: Lighting shader naming must be the provided ones
        light.enabledLoc = GetShaderLocation(shader, TextFormat("lights[%i].enabled", lightsCount));
        light.typeLoc = GetShaderLocation(shader, TextFormat("lights[%i].type", lightsCount));
        light.positionLoc = GetShaderLocation(shader, TextFormat("lights[%i].position", lightsCount));
        light.directionLoc = GetShaderLocation(shader, TextFormat("lights[%i].direction", lightsCount));
        light.colorLoc = GetShaderLocation(shader, TextFormat("lights[%i].color", lightsCount));
        light.radiusLoc = GetShaderLocation(shader, TextFormat("lights[%i].radius", lightsCount));
        light.angleLoc = GetShaderLocation(shader, TextFormat("lights[%i].spotAngle", lightsCount));

        UpdateLightValues(shader, light);
        
        lightsCount++;
    }

    return light;
}

// Send light properties to shader
// NOTE: Light shader locations should be available 
void UpdateLightValues(Shader shader, Light light)
{
    // Send to shader light enabled state and type
    SetShaderValue(shader, light.enabledLoc, &light.enabled, SHADER_UNIFORM_INT);
    SetShaderValue(shader, light.typeLoc, &light.type, SHADER_UNIFORM_INT);

    // Send to shader light position values
    float position[3] = { light.position.x, light.position.y, light.position.z };
    SetShaderValue(shader, light.positionLoc, position, SHADER_UNIFORM_VEC3);

    // Send to shader light target position values
    float target[3] = { light.direction.x, light.direction.y, light.direction.z };
    SetShaderValue(shader, light.directionLoc, target, SHADER_UNIFORM_VEC3);

    // Send to shader light color values
    float color[4] = { (float)light.color.r/(float)255, (float)light.color.g/(float)255, 
                       (float)light.color.b/(float)255, (float)light.color.a/(float)255 };
    SetShaderValue(shader, light.colorLoc, color, SHADER_UNIFORM_VEC4);

    SetShaderValue(shader, light.radiusLoc, &light.radius, SHADER_UNIFORM_FLOAT);
    SetShaderValue(shader, light.angleLoc, &light.angle, SHADER_UNIFORM_FLOAT);

}

void UpdateLightsArray(Shader shader, Light lights[MAX_LIGHTS], Camera cam)
{
    // --- helpers ---
#ifndef RLIGHTS_PI
#define RLIGHTS_PI 3.14159265358979323846f
#endif

// Camera forward (normalized)
    Vector3 fwd = Vector3Normalize(Vector3Subtract(cam.target, cam.position));
    // Half vertical FOV in radians; give ourselves a little slack ( *1.15 )
    float halfFovRad = (cam.fovy * RLIGHTS_PI / 180.0f) * 0.5f * 1.15f;
    float cosHalfFov = cosf(halfFovRad);

    // Collect candidates: indices + distance
    typedef struct { int idx; float dist; int origEnabled; } Candidate;
    Candidate cand[MAX_LIGHTS];
    int candCount = 0;

    for (int i = 0; i < MAX_LIGHTS; ++i) {
        // Skip never-enabled lights
        if (lights[i].enabled == LIGHT_DISABLED) continue;

        // Vector from camera to light
        Vector3 toL = Vector3Subtract(lights[i].position, cam.position);
        float   d2 = toL.x * toL.x + toL.y * toL.y + toL.z * toL.z;
        if (d2 <= 1e-6f) d2 = 1e-6f;
        float   dist = sqrtf(d2);
        Vector3 dir = { toL.x / dist, toL.y / dist, toL.z / dist };

        // Angular visibility: allow some expansion by light radius (bigger lights get more leeway).
        // Expand by asin(radius/dist) but clamp to sensible range.
        float expand = 0.0f;
        if (lights[i].radius > 0.0f) {
            float s = lights[i].radius / (dist + 1e-6f);
            if (s > 1.0f) s = 1.0f;
            expand = asinf(s);                  // radians
        }
        float cosExpanded = cosf(fmaxf(halfFovRad - expand, 0.0f));

        // In front of camera?
        float cosAngle = dir.x * fwd.x + dir.y * fwd.y + dir.z * fwd.z;
        int   visible = (cosAngle >= cosExpanded);

        // Optional distance cull: if the light is way beyond its range from the camera, skip it.
        // This is conservative and cheap.
        if (visible) {
            float allowance = lights[i].radius * 2.0f + 5.0f; // small slack
            if (dist > allowance && lights[i].type != LIGHT_DIRECTIONAL) {
                // keep only if within a loose multiple of its range
                if (dist > lights[i].radius * 6.0f) visible = 0;
            }
        }

        if (visible) {
            cand[candCount].idx = i;
            cand[candCount].dist = dist;
            cand[candCount].origEnabled = lights[i].enabled; // preserve SIMPLE vs VOLUMETRIC
            candCount++;
        }
    }

    // Sort candidates by distance (nearest first) – simple insertion sort (N<=8)
    for (int a = 1; a < candCount; ++a) {
        Candidate key = cand[a];
        int b = a - 1;
        while (b >= 0 && cand[b].dist > key.dist) { cand[b + 1] = cand[b]; b--; }
        cand[b + 1] = key;
    }

    // Decide budget: we can drive up to MAX_LIGHTS but we might want to limit.
    // Keep all visibles (shader already only has MAX_LIGHTS).
    int activeCount = candCount;

    // First disable all lights effectively (don’t mutate uniform slots we won’t use).
    for (int i = 0; i < MAX_LIGHTS; ++i) {
        int disabled = LIGHT_DISABLED;
        if (lights[i].enabled != LIGHT_DISABLED) {
            // Temporarily mark disabled; we’ll re-enable chosen ones below.
            lights[i].enabled = LIGHT_DISABLED;
            SetShaderValue(shader, lights[i].enabledLoc, &disabled, SHADER_UNIFORM_INT);
        }
    }

    // Re-enable the chosen ones (nearest visibles), preserving their original enabled mode
    for (int k = 0; k < activeCount; ++k) {
        int i = cand[k].idx;
        lights[i].enabled = cand[k].origEnabled; // LIGHT_SIMPLE or LIGHT_SIMPLE_AND_VOLUMETRIC
        SetShaderValue(shader, lights[i].enabledLoc, &lights[i].enabled, SHADER_UNIFORM_INT);
    }

    // Push updated per-light values (positions/colors/etc.) for all lights
    // (cheap enough for N<=8; keeps uniforms in sync if anything changed)
    for (int i = 0; i < MAX_LIGHTS; ++i) {
        UpdateLightValues(shader, lights[i]);
    }
}


#endif // RLIGHTS_IMPLEMENTATION