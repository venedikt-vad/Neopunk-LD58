#version 330
// Inputs from vertex shader
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragPosition;
in vec3 fragNormal;

// Uniforms
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec3 viewPos;

#define     MAX_LIGHTS              32

//light types
#define     LIGHT_DIRECTIONAL       0
#define     LIGHT_POINT             1
#define     LIGHT_SPOT              2

//enabled states
#define     LIGHT_DISABLED              0
#define     LIGHT_SIMPLE                1
#define     LIGHT_SIMPLE_AND_VOLUMETRIC 2

struct Light {
    int enabled;
    int type;
    vec3 position;
    vec3 direction; 
    float intensity; // Add support
    vec4 color;
    float radius;         // attenuation/range (also cone height for spot)
    float spotAngle;      // degrees
};

// Lighting uniforms
uniform Light lights[MAX_LIGHTS];
uniform vec4 ambient;

// Volumetrics
uniform float volumetricSteps = 16.0;      // kept for future subdiv if needed
uniform float volumetricIntensity = 0.06;
uniform float volumetricScattering = 0.1;

out vec4 finalColor;

// ------------ Helpers ------------
const float EPSILON = 1e-5;

float attenuationByRadius(float dist, float radius) {
    // Hard cutoff at radius, smooth near the edge
    if (radius <= 0.0) return 0.0;
    float nd = clamp(dist / radius, 0.0, 1.0);
    // inverse-square inside, gentle fade at the end
    float invSq = 1.0 / max(dist*dist, 1e-3);
    float edge  = smoothstep(1.0, 0.8, nd);
    return invSq * edge;
}

// Ray-sphere
bool intersectRaySphere(vec3 rayOrigin, vec3 rayDir, vec3 sphereCenter, float sphereRadius, out float tNear, out float tFar) {
    vec3 L = sphereCenter - rayOrigin;
    float tca = dot(L, rayDir);
    float d2 = dot(L, L) - tca * tca;
    float r2 = sphereRadius * sphereRadius;
    if (d2 > r2) return false;
    float thc = sqrt(max(r2 - d2, 0.0));
    tNear = tca - thc;
    tFar  = tca + thc;
    if (tNear < 0.0) tNear = 0.0;
    if (tFar  < 0.0) return false;
    return true;
}

// Finite cone (apex at light, axis = +coneAxis, height = coneHeight)
bool intersectRayCone(
    vec3 rayOrigin,
    vec3 rayDir,
    vec3 coneApex,
    vec3 coneAxis,
    float cosAngle,     // cos(theta)
    float coneHeight,
    out float tNear,
    out float tFar
) {
    const float EPS = 1e-6;
    const float BIAS = 1e-5;

    vec3 axis = normalize(coneAxis);
    vec3 co   = rayOrigin - coneApex;

    float cos2 = cosAngle * cosAngle;
    float sin2 = max(0.0, 1.0 - cos2);
    float k    = sqrt(max(0.0, sin2 / max(EPS, cos2))); // tan(theta)

    float vd  = dot(rayDir, axis);
    float coV = dot(co, axis);
    float kk1 = 1.0 + k*k;

    float A = dot(rayDir, rayDir) - kk1 * vd * vd;
    float B = 2.0 * (dot(rayDir, co) - kk1 * vd * coV);
    float C = dot(co, co) - kk1 * coV * coV;

    float tHits[3];
    int   hitCount = 0;

    // lateral
    if (abs(A) < EPS) {
        if (abs(B) > EPS) {
            float t = -C / B;
            float z = coV + t * vd;
            if (t >= BIAS && z >= 0.0 && z <= coneHeight)
                tHits[hitCount++] = t;
        }
    } else {
        float disc = B*B - 4.0*A*C;
        if (disc >= 0.0) {
            float s = sqrt(disc);
            float inv2A = 0.5 / A;
            float t0 = (-B - s) * inv2A;
            float t1 = (-B + s) * inv2A;

            float z0 = coV + t0 * vd;
            if (t0 >= BIAS && z0 >= 0.0 && z0 <= coneHeight)
                tHits[hitCount++] = t0;

            float z1 = coV + t1 * vd;
            if (t1 >= BIAS && z1 >= 0.0 && z1 <= coneHeight)
                tHits[hitCount++] = t1;
        }
    }

    // base disk
    vec3 baseCenter = coneApex + axis * coneHeight;
    float denom = dot(rayDir, axis);
    if (abs(denom) > EPS) {
        float tPlane = dot(baseCenter - rayOrigin, axis) / denom;
        if (tPlane >= BIAS) {
            vec3 p = rayOrigin + tPlane * rayDir;
            float r = coneHeight * k;
            if (length(p - baseCenter) <= r + 1e-6)
                tHits[hitCount++] = tPlane;
        }
    }

    if (hitCount == 0) return false;

    bool insideLateral = (C < 0.0) && (coV >= 0.0) && (coV <= coneHeight);
    if (insideLateral) {
        float tMinPos = 1e30;
        for (int i = 0; i < hitCount; ++i) tMinPos = min(tMinPos, tHits[i]);
        if (tMinPos < 1e29) { tNear = 0.0; tFar = tMinPos; return true; }
        return false;
    }

    tNear = tHits[0];
    tFar  = tHits[0];
    for (int i = 1; i < hitCount; ++i) {
        tNear = min(tNear, tHits[i]);
        tFar  = max(tFar, tHits[i]);
    }
    return true;
}

vec3 calculateVolumetricLight_DepthBased(int lightIndex, float intersectionBegin, float intersectionEnd, float fragPositionDepth) {
    float effectiveStart = max(intersectionBegin, 0.0);
    float effectiveEnd   = min(intersectionEnd, fragPositionDepth);
    float effectiveDistance = max(effectiveEnd - effectiveStart, 0.0);

    vec3 lightColor = lights[lightIndex].color.rgb;
    vec3 contribution = lightColor * volumetricIntensity * volumetricScattering * effectiveDistance;

    float distanceToLightCenter = length(lights[lightIndex].position - fragPosition);
    float falloff = exp(-distanceToLightCenter * 0.02);
    contribution *= falloff;

    return clamp(contribution, 0.0, 1.0);
}

// ------------ main ------------
void main() {
    vec4 texelColor = texture(texture0, fragTexCoord);
    
    if (texelColor.a == 0.0) discard;

    // if(texelColor.a < 1.0){
    //     float dithering = (noise1(fragTexCoord.x+fragTexCoord.y*fragTexCoord.x)  + 1.0) /2.0;
    //     if(dithering>texelColor.a) discard;
    // }

    vec3 normal = normalize(fragNormal);
    vec3 viewD = normalize(viewPos - fragPosition);
    vec4 tint = colDiffuse * fragColor;

    // base ambient
    vec4 finalColorBase = texelColor * (ambient / 10.0) * tint;

    for (int i = 0; i < MAX_LIGHTS; i++) {
        if ((lights[i].enabled != LIGHT_SIMPLE) && (lights[i].enabled != LIGHT_SIMPLE_AND_VOLUMETRIC)) continue;

        float I = (lights[i].intensity <= 0.0) ? 1.0 : lights[i].intensity;


        vec3 lightResult = vec3(0.0);

        if (lights[i].type == LIGHT_POINT) {
            float dist = length(lights[i].position - fragPosition);
            float att  = attenuationByRadius(dist, lights[i].radius);
            vec3 L     = normalize(lights[i].position - fragPosition);
            float diff = max(dot(normal, L), 0.0);
            vec3 diffuse  = diff * lights[i].color.rgb;
            vec3 reflectDir = reflect(-L, normal);
            float spec = pow(max(dot(viewD, reflectDir), 0.0), 32.0);
            vec3 specular = spec * lights[i].color.rgb * 0.5;
            lightResult = ((diffuse + specular) * att) * I;
        }
        else if (lights[i].type == LIGHT_SPOT) {
            vec3 Ldir = normalize(lights[i].position - fragPosition);
            vec3 Sdir = normalize(lights[i].direction);            
            float theta = dot(-Ldir, Sdir);                        // angle between to-frag and spot axis
            float cosOuter = cos(radians(lights[i].spotAngle));    // NEW (degrees → radians)
            float cosInner = cos(radians(max(lights[i].spotAngle - 5.0, 0.0))); // small feather (5°)

            float spotFactor = clamp((theta - cosOuter) / max(cosInner - cosOuter, 1e-4), 0.0, 1.0);

            float dist = length(lights[i].position - fragPosition);
            float att  = attenuationByRadius(dist, lights[i].radius);

            float diff = max(dot(normal, Ldir), 0.0);
            vec3 diffuse  = diff * lights[i].color.rgb;
            vec3 reflectDir = reflect(-Ldir, normal);
            float spec = pow(max(dot(viewD, reflectDir), 0.0), 32.0);
            vec3 specular = spec * lights[i].color.rgb * 0.5;

            lightResult = ((diffuse + specular) * att * spotFactor) * I;
        }
        // directional (optional): simple lambert
        else if (lights[i].type == LIGHT_DIRECTIONAL) {
            vec3 L = normalize(-lights[i].direction);
            float diff = max(dot(normal, L), 0.0);
            vec3 diffuse  = diff * lights[i].color.rgb;
            vec3 reflectDir = reflect(-L, normal);
            float spec = pow(max(dot(viewD, reflectDir), 0.0), 32.0);
            vec3 specular = spec * lights[i].color.rgb * 0.25;
            lightResult = (diffuse + specular) * I;
        }

        finalColorBase += texelColor * vec4(lightResult, 0.0) * tint;
    }

    // Volumetrics
    vec3 volumetricLight = vec3(0.0);
    if (volumetricIntensity > 0.001) {
        vec3 rayDir = normalize(fragPosition - viewPos);
        float fragPositionDepth = length(fragPosition - viewPos);

        for (int i = 0; i < MAX_LIGHTS; i++) {
            if (lights[i].enabled != LIGHT_SIMPLE_AND_VOLUMETRIC) continue;

            float t0, t1;
            bool hit = false;

            if (lights[i].type == LIGHT_POINT) {
                hit = intersectRaySphere(viewPos, rayDir, lights[i].position, max(lights[i].radius, 0.0), t0, t1);
            } else if (lights[i].type == LIGHT_SPOT) {
                float cosCut = cos(radians(lights[i].spotAngle));
                // Use radius as cone height/range
                hit = intersectRayCone(viewPos, rayDir, lights[i].position, normalize(lights[i].direction), cosCut, max(lights[i].radius, 0.0), t0, t1);
            }

            if (hit && fragPositionDepth >= t0) {
                //volumetricLight += calculateVolumetricLight_DepthBased(i, t0, t1, fragPositionDepth) * ((lights[i].intensity <= 0.0) ? 0.5 : lights[i].intensity/2.0);
                volumetricLight += calculateVolumetricLight_DepthBased(i, t0, t1, fragPositionDepth);
            }
        }
    }

    vec4 result = finalColorBase + vec4(volumetricLight, 0.0);
    result = clamp(result, 0.0, 2.0);

    // Gamma
    result = pow(result, vec4(1.0/2.2));

    // Fog
    float fogStart = 20.0;
    float fogEnd   = 200.0;
    vec3  fogColor = vec3(0.0);
    float dist = length(viewPos - fragPosition);
    float depth = clamp((dist - fogStart) / (fogEnd - fogStart), 0.0, 1.0);

    float a = texelColor.a * colDiffuse.a * fragColor.a;
    finalColor = vec4(mix(result.rgb, fogColor, depth), a);
}
