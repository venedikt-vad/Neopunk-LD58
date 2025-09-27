#include "raylib.h"
#include "raymath.h"
#include <string>
#include <cstdlib>

#define vec3 Vector3
#define vec2 Vector2

//Gets camera origin and direction
const Ray GetCameraRay(Camera cam);

//Gets actual camera up vector
const vec3 GetCameraUp(Camera cam);
//Gets actual camera right vector
const vec3 GetCameraRight(Camera cam);

const Matrix MakeTransformMatrix(vec3 offset, vec3 rotation, vec3 scale);

const vec3 VectorPlaneProject(vec3 vector, vec3 planeNormal);
const vec3 VectorNormalProject(vec3 vector, vec3 planeNormal);

const vec3 PointPlaneProject(vec3 point, vec3 planeBase, vec3 planeNormal);
const float PointPlaneDistance(vec3 point, vec3 planeBase, vec3 planeNormal);

const std::string FloatToString(float value);
const std::string Vec3ToString(vec3 value);

const Rectangle GetTextureRectangle(Texture tex); 
const vec2 GetTextureSize(Texture tex);

float randomFloat0to1() {
    return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

float randomFloatInRange(float min, float max) {
    if (min > max) {
        float temp = min;
        min = max;
        max = temp;
    }
    return max + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

vec3 Vector3UnitRandom() {
    vec3 v = Vector3RotateByAxisAngle(Vector3UnitX, Vector3UnitZ, randomFloatInRange(0, 360));
    vec3 cross = Vector3CrossProduct(v, Vector3UnitZ);
    v = Vector3RotateByAxisAngle(v, cross, randomFloatInRange(0, 360));

}

vec3 Vector3ConeRandom(vec3 dir, float angle) {
    dir = Vector3Normalize(dir);
    vec3 temp = Vector3UnitY;
    if (Vector3DotProduct(temp, dir) >= 0.95)temp = Vector3UnitZ;
    temp = Vector3CrossProduct(dir, temp);

    vec3 v = Vector3RotateByAxisAngle(dir, temp, randomFloatInRange(-angle, angle));
    v = Vector3RotateByAxisAngle(v, dir, randomFloatInRange(0, 360));

    return v;
}