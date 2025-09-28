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
#define MAX_LIGHTS  32         // Max dynamic lights supported by shader

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

// Light data
typedef struct {   
    int type;
    int enabled;
    Vector3 position;
    Vector3 direction;
    Color color;
    float radius;
    float angle = 45;
    
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

#endif // RLIGHTS_IMPLEMENTATION