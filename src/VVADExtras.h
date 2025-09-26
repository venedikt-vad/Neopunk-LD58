#include "raylib.h"
#include "raymath.h"

#define vec3 Vector3
#define vec2 Vector2

const Ray GetCameraRay(Camera cam);

const Matrix MakeTransformMatrix(vec3 offset, vec3 rotation, vec3 scale);

const vec3 VectorPlaneProject(vec3 vector, vec3 planeNormal);  //Projects vector on to plane
const vec3 VectorNormalProject(vec3 vector, vec3 planeNormal); //Projects vector on to normal
