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

//do not add new fields while prototyping
struct Light {
    int enabled;
    int type;
    vec3 position;
    vec3 target;
    vec4 color;
};

// Input lighting values
uniform Light lights[MAX_LIGHTS];
uniform vec4 ambient;

// Volumetric light parameters
uniform float volumetricSteps = 16.0;
uniform float volumetricIntensity = 0.1; // Much smaller value
uniform float volumetricScattering = 0.1; // Much smaller value

// Function to calculate volumetric light contribution
vec3 calculateVolumetricLight(vec3 rayStart, vec3 rayEnd, int lightIndex) {
    vec3 lightDir = normalize(rayEnd - rayStart);
    float rayLength = length(rayEnd - rayStart);
    
    if (rayLength < 0.001) return vec3(0.0); // Avoid division by zero
    
    vec3 rayStep = lightDir * (rayLength / max(volumetricSteps, 1.0));
    
    vec3 totalLight = vec3(0.0);
    vec3 currentPos = rayStart;
    
    // Pre-calculate light properties for this light
    vec3 lightColor = lights[lightIndex].color.rgb;
    float maxDistance = 50.0; // Maximum effective distance for volumetrics
    
    for (int i = 0; i < int(volumetricSteps); i++) {
        float distanceToLight = length(lights[lightIndex].position - currentPos);
        
        // Early exit if too far from light
        if (distanceToLight > maxDistance) {
            currentPos += rayStep;
            continue;
        }
        
        float lightInfluence = 0.0;
        
        if (lights[lightIndex].type == LIGHT_POINT) {
            // Point light with tighter, more controlled falloff
            float lightAttenuation = 1.0 / (1.0 + 0.1 * distanceToLight + 0.032 * distanceToLight * distanceToLight);
            float distanceFactor = max(0.0, 1.0 - (distanceToLight / maxDistance));
            lightInfluence = lightAttenuation * distanceFactor * distanceFactor; // Square for sharper falloff
        }
        else if (lights[lightIndex].type == LIGHT_SPOT) {
            // Spot light volumetric contribution
            vec3 lightToPos = normalize(lights[lightIndex].position - currentPos);
            vec3 spotDir = normalize(-lights[lightIndex].target);
            
            float theta = dot(lightToPos, spotDir);
            float innerCone = 0.9;  // Tighter cone
            float outerCone = 0.7;
            float epsilon = innerCone - outerCone;
            float intensity = clamp((theta - outerCone) / epsilon, 0.0, 1.0);
            
            if (intensity > 0.0) {
                float lightAttenuation = 1.0 / (1.0 + 0.09 * distanceToLight + 0.032 * distanceToLight * distanceToLight);
                lightInfluence = intensity * lightAttenuation;
            }
        }
        
        // Accumulate light with clamping to prevent overflow
        vec3 stepContribution = lightColor * lightInfluence * volumetricIntensity * volumetricScattering;
        totalLight += clamp(stepContribution, 0.0, 0.1); // Clamp individual contributions
        
        currentPos += rayStep;
    }
    
    return clamp(totalLight, 0.0, 1.0); // Final clamp
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

    // Calculate volumetric lighting only for pixels that are not directly on geometry
    // This is a simple approach - in reality, you'd want a separate pass or screen-space technique
    vec3 volumetricLight = vec3(0.0);
    
    // Only calculate volumetrics if we're not on a surface (this is a simplification)
    // In a real implementation, you'd have a separate volumetric pass
    if (volumetricIntensity > 0.001) {
        for (int i = 0; i < MAX_LIGHTS; i++) {
            if (lights[i].enabled == 1) {
                // Calculate ray from camera to fragment position
                vec3 rayStart = viewPos;
                vec3 rayEnd = fragPosition;
                
                // Add volumetric contribution from this light
                vec3 volLight = calculateVolumetricLight(rayStart, rayEnd, i);
                
                // Apply distance-based falloff to prevent overwhelming the scene
                float distanceToCamera = length(viewPos - fragPosition);
                float falloff = exp(-distanceToCamera * 0.02);
                
                volumetricLight += volLight * falloff;
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