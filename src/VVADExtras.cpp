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
