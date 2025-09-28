#version 330
// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragPosition;
in vec3 fragNormal;
// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;
// Output fragment color
out vec4 finalColor;
// NOTE: Add here your custom variables
uniform vec3 viewPos;
#define     MAX_LIGHTS              4
#define     LIGHT_DIRECTIONAL       0
#define     LIGHT_POINT             1
#define     LIGHT_SPOT              2

struct Light {
    int enabled;
    int type;
    vec3 position;
    vec3 target; // For spot lights, represents the direction
    vec4 color;
    float range; // Add range for point lights to define volume size
    float spotAngle; // Add angle for spot lights (cosine of inner angle)
};

// Input lighting values
uniform Light lights[MAX_LIGHTS];
uniform vec4 ambient;

// Volumetric light parameters
uniform float volumetricSteps = 16.0; // Might not be needed for this approach, but keep if you want to subdivide the calculated distance
uniform float volumetricIntensity = 0.1;
uniform float volumetricScattering = 0.1;
// Add a parameter for light volume size (e.g., radius for point light, cone angle for spot light)
uniform float pointLightVolumeRadius = 10.0; // Example default radius
uniform float spotLightCutoffAngle = 0.785; // Example default cutoff angle (45 degrees in radians)


// Function to calculate ray-sphere intersection (for point lights)
// Returns true if hit, fills tNear and tFar with intersection distances along the ray
// Ray: origin = rayOrigin, direction = rayDir (should be normalized)
// Sphere: center = sphereCenter, radius = sphereRadius
bool intersectRaySphere(vec3 rayOrigin, vec3 rayDir, vec3 sphereCenter, float sphereRadius, out float tNear, out float tFar) {
    vec3 L = sphereCenter - rayOrigin;
    float tca = dot(L, rayDir);
    float dSquared = dot(L, L) - tca * tca;
    float radiusSquared = sphereRadius * sphereRadius;

    if (dSquared > radiusSquared) return false; // Ray misses the sphere

    float thc = sqrt(radiusSquared - dSquared);
    tNear = tca - thc;
    tFar = tca + thc;

    // If tNear is behind the ray origin, the ray starts inside the sphere
    if (tNear < 0.0) tNear = 0.0;

    // If both are negative, the ray points away from the sphere
    if (tFar < 0.0) return false;

    return true;
}

// Function to calculate ray-cone intersection (for spot lights)
// Simplified: Assumes cone axis is along -target direction, apex at light position
// Returns true if hit, fills tNear and tFar with intersection distances along the ray
// This is a more complex intersection, this is a basic example assuming infinite cone
// You might need to clip it at a certain range.
bool intersectRayCone(vec3 rayOrigin, vec3 rayDir, vec3 coneApex, vec3 coneAxis, float cosAngle, out float tNear, out float tFar) {
    // Simplified infinite cone calculation
    // coneAxis should be normalized (direction from apex)
    // cosAngle is cosine of the half-angle of the cone
    vec3 V = rayOrigin - coneApex;
    float a = dot(rayDir, coneAxis);
    float b = dot(V, coneAxis);
    float c = dot(rayDir, rayDir);
    float d = dot(V, rayDir);
    float e = dot(V, V);

    float cosAngleSq = cosAngle * cosAngle;

    // Coefficients for quadratic equation At^2 + Bt + C = 0
    float A = c * cosAngleSq - a * a;
    float B = 2.0 * (d * cosAngleSq - a * b);
    float C = e * cosAngleSq - b * b;

    // Solve quadratic
    float discriminant = B * B - 4.0 * A * C;

    if (discriminant < 0.0) return false; // No intersection

    float sqrtDiscriminant = sqrt(discriminant);
    float t1 = (-B - sqrtDiscriminant) / (2.0 * A);
    float t2 = (-B + sqrtDiscriminant) / (2.0 * A);

    // Ensure t1 <= t2
    if (t1 > t2) { float temp = t1; t1 = t2; t2 = temp; }

    // Check if intersections are in front of ray origin
    if (t2 < 0.0) return false; // Both intersections are behind

    // Check if the ray starts inside the cone volume (optional, set tNear accordingly)
    vec3 startToApex = rayOrigin - coneApex;
    vec3 startDir = normalize(startToApex);
    float startCosTheta = dot(startDir, coneAxis);
    bool startInside = startCosTheta > cosAngle; // Check if starting point is inside the infinite cone

    if (startInside) {
        tNear = 0.0; // Start inside, so near point is 0
        tFar = max(t1, t2); // Take the further intersection as the exit
        if (tFar < 0.0) return false; // Exit is also behind
    } else {
        tNear = t1;
        tFar = t2;
        if (tNear < 0.0) tNear = 0.0; // Start outside, clip near to 0 if behind origin
        if (tFar < 0.0) return false; // Exit is behind origin
    }

    // Optional: Clip to light range if needed
    // float lightRange = lights[lightIndex].range; // Use if range is available in struct
    // if (tNear > lightRange || tFar < 0.0 /* or tNear > tFar*/) return false;
    // tNear = min(tNear, lightRange);
    // tFar = min(tFar, lightRange);

    return true;
}


// Function to calculate volumetric light contribution based on intersection depths
// This is a simplified version. You might want to integrate density along the distance differently.
vec3 calculateVolumetricLight_DepthBased(int lightIndex, float intersectionBegin, float intersectionEnd, float fragPositionDepth) {
    // 5. Calculate volumetrics distance
    // The effective distance through the volume the fragment "sees"
    // This is the distance along the camera ray within the light volume
    // that is *in front of* the camera and *behind* the visible geometry (fragPositionDepth)
    float effectiveStart = max(intersectionBegin, 0.0); // Ensure start is not behind camera
    float effectiveEnd = min(intersectionEnd, fragPositionDepth); // End is either volume exit or geometry depth
    float effectiveDistance = max(effectiveEnd - effectiveStart, 0.0); // Ensure positive distance

    // 6. Apply volumetrics to the final color
    // This is a very basic calculation. You could use volumetricSteps to subdivide this distance
    // and apply a more complex density/integration model.
    vec3 lightColor = lights[lightIndex].color.rgb;
    vec3 contribution = lightColor * volumetricIntensity * volumetricScattering * effectiveDistance;

    // Optional: Apply falloff based on distance from light center or camera
    float distanceToLightCenter = length(lights[lightIndex].position - fragPosition);
    float falloff = exp(-distanceToLightCenter * 0.02); // Example exponential falloff
    contribution *= falloff;

    return clamp(contribution, 0.0, 1.0); // Clamp to prevent extreme values
}


void main() {
    vec4 texelColor = texture(texture0, fragTexCoord);
    vec3 normal = normalize(fragNormal);
    vec3 viewD = normalize(viewPos - fragPosition);
    vec4 tint = colDiffuse * fragColor;

    // Calculate base lighting (your original lighting)
    vec4 finalColorBase = texelColor * (ambient / 10.0) * tint; // ambient light
    for (int i = 0; i < MAX_LIGHTS; i++) {
        if (lights[i].enabled == 1) {
            vec3 lightResult = vec3(0.0);
            if (lights[i].type == LIGHT_POINT) {
                lightResult = mix(lights[i].color, vec4(0,0,0,1), min(length(lights[i].position - fragPosition)/3,1)).rgb;
            }
            else if (lights[i].type == LIGHT_SPOT) {
                // Spot light calculation
                vec3 lightDir = normalize(lights[i].position - fragPosition);
                vec3 spotDir = normalize(-lights[i].target);
                float theta = dot(lightDir, spotDir);
                float epsilon = 0.1;
                float intensity = clamp((theta - 0.8) / epsilon, 0.0, 1.0);
                if (intensity > 0.0) {
                    float distance = length(lights[i].position - fragPosition);
                    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
                    float diff = max(dot(normal, lightDir), 0.0);
                    vec3 diffuse = diff * lights[i].color.rgb;
                    vec3 reflectDir = reflect(-lightDir, normal);
                    float spec = pow(max(dot(viewD, reflectDir), 0.0), 32.0);
                    vec3 specular = spec * lights[i].color.rgb * 0.5;
                    lightResult = (diffuse + specular) * attenuation * intensity;
                }
            }
            finalColorBase += texelColor * vec4(lightResult, 0.0) * tint;
        }
    }

    // Calculate volumetric lighting based on depth intersection
    vec3 volumetricLight = vec3(0.0);

    if (volumetricIntensity > 0.001) {
        // Get the camera ray direction (direction from camera to fragment, normalized)
        vec3 rayDir = normalize(fragPosition - viewPos);
        // Calculate the depth of the current fragment (distance from camera)
        float fragPositionDepth = length(fragPosition - viewPos);

        for (int i = 0; i < MAX_LIGHTS; i++) {
            if (lights[i].enabled == 1) {
                float intersectionBegin = -1.0;
                float intersectionEnd = -1.0;
                bool intersects = false;

                // 1. Check if current pixel ray intersects light volume
                if (lights[i].type == LIGHT_POINT) {
                    // Use the defined volume radius for point lights
                    float lightRadius = pointLightVolumeRadius; // Use uniform or potentially lights[i].range if added to struct
                    intersects = intersectRaySphere(viewPos, rayDir, lights[i].position, lightRadius, intersectionBegin, intersectionEnd);
                } else if (lights[i].type == LIGHT_SPOT) {
                    // Use the defined cutoff angle for spot lights
                    float cosCutoff = cos(spotLightCutoffAngle); // Calculate cosine once
                    vec3 coneAxis = normalize(-lights[i].target); // Assuming target defines direction
                    intersects = intersectRayCone(viewPos, rayDir, lights[i].position, coneAxis, cosCutoff, intersectionBegin, intersectionEnd);
                }
                // Add other light types if needed (e.g., directional box?)

                // 2. If intersects, get intersection begin and end points (already calculated in 'intersects' call)
                if (intersects) {
                    // 3. Calculate pixel depth from fragPosition (already calculated as fragPositionDepth)

                    // 4. If fragPosition closer than intersectionBegin, do not draw volumetrics
                    if (fragPositionDepth >= intersectionBegin) {
                         // 5. Calculate volumetrics distance and 6. apply to final color
                         vec3 volLight = calculateVolumetricLight_DepthBased(i, intersectionBegin, intersectionEnd, fragPositionDepth);
                         volumetricLight += volLight;
                    }
                    // else: fragment is in front of the light volume, no volumetrics
                }
                // else: no intersection, no volumetrics from this light
            }
        }
    }

    // Combine base lighting with volumetric lighting
    vec4 result = finalColorBase + vec4(volumetricLight, 0.0);
    // Ensure we don't exceed reasonable values
    result = clamp(result, 0.0, 2.0);

    // Gamma correction
    finalColor = pow(result, vec4(1.0/2.2));

    // Fog
    float fogStart = 20.0;
    float fogEnd = 200.0;
    vec3 fogColor = vec3(1.,1.,1.);
    float dist = length(viewPos - fragPosition);
    float depth = clamp((dist - fogStart) / (fogEnd - fogStart), 0.0, 1.0);
    finalColor = vec4(mix(finalColor.rgb, fogColor, depth), 1.0f);
}