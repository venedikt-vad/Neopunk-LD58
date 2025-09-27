#include "VVADExtras.h"

const Ray GetCameraRay(Camera cam) {
    return { cam.position, Vector3Normalize({ cam.target - cam.position }) };
}

const vec3 GetCameraUp(Camera cam) {
    return Vector3CrossProduct(GetCameraRight(cam), cam.target - cam.position);
}

const vec3 GetCameraRight(Camera cam) {

    return Vector3CrossProduct(cam.target - cam.position, cam.up);
}



const Matrix MakeTransformMatrix(vec3 offset, vec3 rotation, vec3 scale) {
    Matrix matScale = MatrixScale(scale.x, scale.y, scale.z);
    Matrix matRotation = MatrixRotateXYZ(rotation * DEG2RAD);
    Matrix matTranslation = MatrixTranslate(offset.x, offset.y, offset.z);
    Matrix result = MatrixMultiply(MatrixMultiply(matScale, matRotation), matTranslation);
    return result;
};

const vec3 VectorPlaneProject(vec3 vector, vec3 planeNormal) {
    // Normalize the plane normal to ensure it's a unit vector
    vec3 normalizedNormal = Vector3Normalize(planeNormal);

    // Project vector onto normal
    vec3 normalProjection = VectorNormalProject(vector, normalizedNormal);

    // Subtract normal projection from original vector to get plane projection
    return Vector3Subtract(vector, normalProjection);
}

const vec3 VectorNormalProject(vec3 vector, vec3 planeNormal) {
    // Normalize the plane normal to ensure it's a unit vector
    vec3 normalizedNormal = Vector3Normalize(planeNormal);

    // Calculate dot product of vector and normalized normal
    float dotProduct = Vector3DotProduct(vector, normalizedNormal);

    // Project vector onto normal: (v·n) * n
    return Vector3Scale(normalizedNormal, dotProduct);
}

const vec3 PointPlaneProject(vec3 point, vec3 planeBase, vec3 planeNormal) {
    // Normalize the plane normal
    vec3 normalizedNormal = Vector3Normalize(planeNormal);

    // Vector from plane base to the point
    vec3 pointToPlane = Vector3Subtract(point, planeBase);

    // Project this vector onto the plane normal
    vec3 normalProjection = VectorNormalProject(pointToPlane, normalizedNormal);

    // Subtract the normal projection from the original point to get the projection on the plane
    return Vector3Subtract(point, normalProjection);
}

const float PointPlaneDistance(vec3 point, vec3 planeBase, vec3 planeNormal) {
    // Normalize the plane normal
    vec3 normalizedNormal = Vector3Normalize(planeNormal);

    // Vector from plane base to the point
    vec3 pointToPlane = Vector3Subtract(point, planeBase);

    // The distance is the absolute value of the projection onto the normal
    float distance = Vector3DotProduct(pointToPlane, normalizedNormal);

    return fabsf(distance);
}

const std::string FloatToString(float value) {
    return std::to_string(value);
}

const std::string Vec3ToString(vec3 value) {
    std::string s = "x: " + std::to_string(value.x) + " y: " + std::to_string(value.y) + " z: " + std::to_string(value.z);
    return s;
}

const Rectangle GetTextureRectangle(Texture tex){
    return {0,0, (float)tex.width, (float)tex.height};
}

const vec2 GetTextureSize(Texture tex) {
    return {(float)tex.width, (float)tex.height };
}


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
    return v;
}

vec3 Vector3ConeRandom(vec3 dir, float angle) {
    angle = angle * DEG2RAD;
    dir = Vector3Normalize(dir);
    vec3 temp = Vector3UnitY;
    if (Vector3DotProduct(temp, dir) >= 0.95)temp = Vector3UnitZ;
    temp = Vector3CrossProduct(dir, temp);

    vec3 v = Vector3RotateByAxisAngle(dir, temp, randomFloatInRange(-angle, angle));
    v = Vector3RotateByAxisAngle(v, dir, randomFloatInRange(0, 360));

    return v;
}