#include "VVADExtras.h"

const Ray GetCameraRay(Camera cam) {
    return { cam.position, Vector3Normalize({ cam.target - cam.position }) };
}

const vec3 GetCameraUp(Camera cam) {
    return Vector3Normalize(Vector3CrossProduct(GetCameraRight(cam), cam.target - cam.position));
}

const vec3 GetCameraRight(Camera cam) {
    return Vector3Normalize(Vector3CrossProduct(cam.target - cam.position, cam.up));
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

    // Project vector onto normal: (v�n) * n
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

    if (min == max) {
        return min;
    }

    return min + (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * (max - min);
}

vec3 Vector3UnitRandom() {
    vec3 v = Vector3RotateByAxisAngle(Vector3UnitX, Vector3UnitZ, randomFloatInRange(0, 360));
    vec3 cross = Vector3CrossProduct(v, Vector3UnitZ);
    v = Vector3RotateByAxisAngle(v, cross, randomFloatInRange(0, 360));
    return v;
}

vec3 Vector3RandomInVolume(vec3 volume) {
    vec3 v = {
        randomFloatInRange(0, volume.x),
        randomFloatInRange(0, volume.y),
        randomFloatInRange(0, volume.z)
    };
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

Matrix TransformToMatrix(Transform transform) {
    //Temporary transform matrix
    Matrix _tfMatrix = MatrixIdentity();	//transformMatrix

    //Matrix scale (in local space)
    Matrix _scMatrix = MatrixScale(transform.scale.x, transform.scale.y, transform.scale.z); //scaleMatrix
    _tfMatrix = MatrixMultiply(_tfMatrix, _scMatrix);	//applyScale

    //Convert Quaternion to rotation matrix
    Matrix _rtMatrix = QuaternionToMatrix(transform.rotation);	//rotationMatrix

    //matrix rotation
    Matrix _trMatrix = MatrixIdentity();	//translationMatrix
    if (transform.translation.x != 0 || transform.translation.y != 0 || transform.translation.z != 0)
        _trMatrix = MatrixTranslate(transform.translation.x, transform.translation.y, transform.translation.z);

    //Finalize transform
    _tfMatrix = MatrixMultiply(_tfMatrix, _rtMatrix);	//applyRotation
    _tfMatrix = MatrixMultiply(_tfMatrix, _trMatrix);	//applyTransform

    return _tfMatrix;
}

Quaternion QuaternionFromForward(Vector3 forward) {
    // 1) Защита от нулевого направления
    if (Vector3LengthSqr(forward) < 1e-12f) return QuaternionIdentity();

    // 2) Нормализуем forward
    forward = Vector3Normalize(forward);

    // 3) Базовый мировой up = Z, если почти параллелен — запасной = Y
    Vector3 up = (fabsf(Vector3DotProduct(forward,  { 0, 0, 1 })) > 0.999f) ? Vector3UnitY : Vector3UnitZ;

    // 4) Праворукий базис
    Vector3 right = Vector3Normalize(Vector3CrossProduct(up, forward));  // right = up × forward
    up = Vector3CrossProduct(forward, right);                            // up = forward × right (уже нормализован)

    // 5) Матрица: колонки = right, up, forward (OpenGL/raylib column-major)
    Matrix m = {
        right.x,   up.x,      forward.x, 0.0f,   // m0  m4  m8  m12
        right.y,   up.y,      forward.y, 0.0f,   // m1  m5  m9  m13
        right.z,   up.z,      forward.z, 0.0f,   // m2  m6  m10 m14
        0.0f,      0.0f,      0.0f,      1.0f    // m3  m7  m11 m15
    };

    return QuaternionFromMatrix(m);
}

Vector3 GetForwardVector(const Transform &transform) {
    Matrix rot = QuaternionToMatrix(transform.rotation);
    Vector3 forward = {0.0f, 0.0f, 1.0f};
    forward = Vector3Transform(forward, rot);
    return Vector3Normalize(forward);
}

static bool PointInTriBary(Vector3 P, Vector3 A, Vector3 B, Vector3 C, float eps)
{
    Vector3 v0 = Vector3Subtract(C, A);
    Vector3 v1 = Vector3Subtract(B, A);
    Vector3 v2 = Vector3Subtract(P, A);

    float dot00 = Vector3DotProduct(v0, v0);
    float dot01 = Vector3DotProduct(v0, v1);
    float dot02 = Vector3DotProduct(v0, v2);
    float dot11 = Vector3DotProduct(v1, v1);
    float dot12 = Vector3DotProduct(v1, v2);

    float denom = dot00 * dot11 - dot01 * dot01;
    if (fabsf(denom) < 1e-8f) return false;

    float invDen = 1.0f / denom;
    float u = (dot11 * dot02 - dot01 * dot12) * invDen;
    float v = (dot00 * dot12 - dot01 * dot02) * invDen;

    return (u >= -eps) && (v >= -eps) && (u + v <= 1.0f + eps);
}

// Ray vs finite capsule [a,b] with radius r (cylinder + two cap spheres via your API).
// Uses normalized ray; returns t along that normalized ray.
// Filters out hits where the sphere center at contact is on the plane's BACK side (front-face policy).
static bool RayCapsuleHitFrontOnly(
    Ray rayN, Vector3 a, Vector3 b, float r,
    Vector3 triP, Vector3 triN,           // plane anchor and normal
    float* tOut, Vector3* nOut, Vector3* contactOut)
{
    Vector3 ab = Vector3Subtract(b, a);
    float abLen = Vector3Length(ab);
    if (abLen < 1e-6f) {
        // Degenerate: just check sphere at 'a'
        RayCollision rc = GetRayCollisionSphere(rayN, a, r);
        if (!rc.hit) return false;
        // Front-face filter:
        Vector3 centerAt = Vector3Add(rayN.position, Vector3Scale(rayN.direction, rc.distance));
        float d = Vector3DotProduct(triN, Vector3Subtract(centerAt, triP));
        if (d < -1e-5f) return false;

        if (tOut) *tOut = rc.distance;
        if (nOut) *nOut = rc.normal;
        if (contactOut) *contactOut = a;
        return true;
    }

    Vector3 u = Vector3Scale(ab, 1.0f / abLen); // axis unit
    Vector3 w = Vector3Subtract(rayN.position, a);
    float vPar = Vector3DotProduct(rayN.direction, u);
    float wPar = Vector3DotProduct(w, u);

    // Perpendicular components to the axis
    Vector3 vPerp = Vector3Subtract(rayN.direction, Vector3Scale(u, vPar));
    Vector3 wPerp = Vector3Subtract(w, Vector3Scale(u, wPar));

    float A = Vector3DotProduct(vPerp, vPerp);
    float B = 2.0f * Vector3DotProduct(wPerp, vPerp);
    float C = Vector3DotProduct(wPerp, wPerp) - r * r;

    float bestT = FLT_MAX;
    Vector3 bestN = Vector3Zeros;
    Vector3 bestC = Vector3Zeros;
    bool hit = false;

    // Infinite cylinder intersection
    if (A > 1e-8f) {
        float disc = B * B - 4.0f * A * C;
        if (disc >= 0.0f) {
            float s = sqrtf(disc);
            float t0 = (-B - s) / (2.0f * A);
            float t1 = (-B + s) / (2.0f * A);

            for (int k = 0; k < 2; k++) {
                float t = (k == 0) ? t0 : t1;
                if (t < 0.0f) continue;

                float sAxis = wPar + vPar * t; // scalar along [0,abLen]
                if (sAxis >= 0.0f && sAxis <= abLen) {
                    Vector3 centerAt = Vector3Add(rayN.position, Vector3Scale(rayN.direction, t));
                    // Front-face filter: center must be on or in front of triangle plane
                    float d = Vector3DotProduct(triN, Vector3Subtract(centerAt, triP));
                    if (d < -1e-5f) continue;

                    Vector3 Q = Vector3Add(a, Vector3Scale(u, sAxis)); // foot on edge
                    Vector3 N = Vector3Normalize(Vector3Subtract(centerAt, Q)); // radial normal

                    if (t < bestT) { bestT = t; bestN = N; bestC = Q; hit = true; }
                }
            }
        }
    }

    // Cap spheres via your API, with front-face filter
    RayCollision rca = GetRayCollisionSphere(rayN, a, r);
    if (rca.hit) {
        Vector3 centerAt = Vector3Add(rayN.position, Vector3Scale(rayN.direction, rca.distance));
        float d = Vector3DotProduct(triN, Vector3Subtract(centerAt, triP));
        if (d >= -1e-5f && rca.distance < bestT) { bestT = rca.distance; bestN = rca.normal; bestC = a; hit = true; }
    }
    RayCollision rcb = GetRayCollisionSphere(rayN, b, r);
    if (rcb.hit) {
        Vector3 centerAt = Vector3Add(rayN.position, Vector3Scale(rayN.direction, rcb.distance));
        float d = Vector3DotProduct(triN, Vector3Subtract(centerAt, triP));
        if (d >= -1e-5f && rcb.distance < bestT) { bestT = rcb.distance; bestN = rcb.normal; bestC = b; hit = true; }
    }

    if (!hit) return false;
    if (tOut) *tOut = bestT;
    if (nOut) *nOut = bestN;
    if (contactOut) *contactOut = bestC;
    return true;
}

SphereTraceCollision GetSphereTraceCollisionTriangle(Ray inRay, float sphereRadius, Vector3 p1, Vector3 p2, Vector3 p3)
{
#define EPSILON   1e-6f
#define FACE_EPS  1e-3f
    SphereTraceCollision col = { 0 };

    // Normalize ray; we�ll convert t back to world units
    float dirLen = Vector3Length(inRay.direction);
    if (dirLen < EPSILON) return col;
    Ray rayN = inRay;
    rayN.direction = Vector3Scale(inRay.direction, 1.0f / dirLen);

    // Triangle normal (unit)
    Vector3 e1 = Vector3Subtract(p2, p1);
    Vector3 e2 = Vector3Subtract(p3, p1);
    Vector3 n = Vector3CrossProduct(e1, e2);
    float nLen = Vector3Length(n);
    if (nLen < EPSILON) return col;
    n = Vector3Scale(n, 1.0f / nLen);

    // Front-face only: require ray to approach the front side (ndot < 0)
    float ndot = Vector3DotProduct(n, rayN.direction);
    if (ndot >= -EPSILON) return col; // backface or parallel away: ignore

    // Signed distance of current center to plane
    float d0 = Vector3DotProduct(n, Vector3Subtract(rayN.position, p1));

    // --- INITIAL OVERLAP (t == 0), face only ---
    if (d0 >= -1e-5f && d0 <= sphereRadius + 1e-5f) { // center on or in front; within radius slab
        // Project current center to plane and test inside triangle
        Vector3 proj = Vector3Subtract(rayN.position, Vector3Scale(n, d0));
        if (PointInTriBary(proj, p1, p2, p3, FACE_EPS)) {
            // Push sphere center along surface normal so it's just touching
            float push = fmaxf(0.0f, sphereRadius - d0);
            Vector3 correctedCenter = Vector3Add(rayN.position, Vector3Scale(n, push));

            col.hit = true;
            col.initialHit = true;
            col.distance = 0.0f;     // as requested
            col.normal = n;          // face normal
            col.point = correctedCenter;
            col.contactPoint = proj; // on the triangle plane
            return col;
        }
    }

    // --- FACE HIT (front-face only) ---
    // Solve d(t) = R  with d(t) = d0 + t*ndot  and ndot < 0  ->  t = (R - d0)/ndot
    float tFace = (sphereRadius - d0) / ndot;
    if (tFace >= 0.0f) {
        Vector3 centerAt = Vector3Add(rayN.position, Vector3Scale(rayN.direction, tFace));
        // True contact point on plane via projection (robust against FP)
        float dAt = Vector3DotProduct(n, Vector3Subtract(centerAt, p1));
        Vector3 contactOnPlane = Vector3Subtract(centerAt, Vector3Scale(n, dAt));

        if (PointInTriBary(contactOnPlane, p1, p2, p3, FACE_EPS)) {
            col.hit = true;
            col.distance = tFace * dirLen; // convert back
            col.normal = n;
            col.point = centerAt;          // sphere center at first touch
            col.contactPoint = contactOnPlane;
            return col; // earliest hit wins; face beats edges
        }
    }

    return col;
}

const SphereTraceCollision GetSphereTraceCollisionMesh(Ray ray, float sphereRadius, Mesh mesh, Matrix transform) {
    SphereTraceCollision collision = { 0 };

    // Check if mesh vertex data on CPU for testing
    if (mesh.vertices != NULL) {
        int triangleCount = mesh.triangleCount;

        // Test against all triangles in mesh
        for (int i = 0; i < triangleCount; i++) {
            Vector3 a, b, c;
            Vector3* vertdata = (Vector3*)mesh.vertices;

            if (mesh.indices) {
                a = vertdata[mesh.indices[i * 3 + 0]];
                b = vertdata[mesh.indices[i * 3 + 1]];
                c = vertdata[mesh.indices[i * 3 + 2]];
            } else {
                a = vertdata[i * 3 + 0];
                b = vertdata[i * 3 + 1];
                c = vertdata[i * 3 + 2];
            }

            a = Vector3Transform(a, transform);
            b = Vector3Transform(b, transform);
            c = Vector3Transform(c, transform);

            SphereTraceCollision triHitInfo = GetSphereTraceCollisionTriangle(ray, sphereRadius, a, b, c);

            if (triHitInfo.hit) {
                // Save the closest hit triangle
                if ((!collision.hit) || (collision.distance > triHitInfo.distance)) collision = triHitInfo;
            }
        }
    }

    return collision;
}