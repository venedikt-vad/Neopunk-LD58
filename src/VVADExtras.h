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

float randomFloat0to1();

float randomFloatInRange(float min, float max);

vec3 Vector3UnitRandom();

vec3 Vector3ConeRandom(vec3 dir, float angle);