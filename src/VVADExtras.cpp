#include "VVADExtras.h"

const Ray GetCameraRay(Camera cam) {
    return { cam.position, Vector3Normalize({ cam.target - cam.position }) };
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