#pragma once
#include "raylib.h"
#include "raymath.h"
#include <string>
#include <cstdlib>

#define vec3 Vector3
#define vec2 Vector2

typedef struct {
    bool hit;
    bool initialHit;
    float distance;
    Vector3 normal;
    Vector3 point;          // Point where sphere center is at collision
    Vector3 contactPoint;   // Point where sphere touches the triangle
} SphereTraceCollision;

typedef struct {
    int x;
    int y;
} iVec2;

typedef struct {
    int x;
    int y;
    int z;
} iVec3;

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

static inline float Clamp01(float x) { return fmaxf(0.0f, fminf(1.0f, x)); }

float randomFloat0to1();

float randomFloatInRange(float min, float max);

vec3 Vector3UnitRandom();

vec3 Vector3RandomInVolume(vec3 volume);

vec3 Vector3ConeRandom(vec3 dir, float angle);

//Thanks to Boyfinn https://github.com/raysan5/raylib/discussions/3670
Matrix TransformToMatrix(Transform transform);
//--

static bool PointInTriBary(Vector3 P, Vector3 A, Vector3 B, Vector3 C, float eps);

static bool RayCapsuleHitFrontOnly(
    Ray rayN, Vector3 a, Vector3 b, float r,
    Vector3 triP, Vector3 triN,
    float* tOut, Vector3* nOut, Vector3* contactOut);

SphereTraceCollision GetSphereTraceCollisionTriangle(Ray ray, float sphereRadius, Vector3 p1, Vector3 p2, Vector3 p3);
const SphereTraceCollision GetSphereTraceCollisionMesh(Ray ray, float sphereRadius, Mesh mesh, Matrix transform);