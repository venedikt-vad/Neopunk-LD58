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

void main() {
    vec4 texelColor = texture(texture0, fragTexCoord);
    vec3 lightDot = vec3(0.0);
    vec3 normal = normalize(fragNormal);
    vec3 viewD = normalize(viewPos - fragPosition);
    vec3 specular = vec3(0.5);

    vec4 noLightColor = vec4(0,0,0,1);

    vec4 tint = colDiffuse * fragColor;

    //SimpleLights

    finalColor = texelColor*(ambient/10.0)*tint; //ambient light

    vec4 lightCol = vec4(0,0,0,0);
    
    for (int i = 0; i < MAX_LIGHTS; i++) {
        if (lights[i].enabled == 1) {

            vec3 lightResult = vec3(0.0);

            //don't need directional light for now
            // if (lights[i].type == LIGHT_DIRECTIONAL) {
            //     //calculation for directional light
            // }

            if (lights[i].type == LIGHT_POINT) {
                lightResult = mix(lights[i].color, noLightColor, min(length(lights[i].position - fragPosition)/3,1)).rgb;
                
                
            }

            if (lights[i].type == LIGHT_SPOT) {
                //calculation for the spot light
                vec3 lightDir = normalize(lights[i].position - fragPosition);
                vec3 spotDir = normalize(-lights[i].target);
                
                float theta = dot(lightDir, spotDir);
                float epsilon = 0.1; // Inner angle cosine
                float intensity = clamp((theta - 0.8) / epsilon, 0.0, 1.0); // Outer angle cosine = 0.7
                
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

            finalColor += texelColor * vec4(lightResult, 0.0) * tint;
            
        }
    }

    // Gamma correction
    finalColor = pow(finalColor, vec4(1.0/2.2));


    //Fog
    float fogStart = 20.0;
    float fogEnd = 200.0;

    vec3 fogColor = vec3(1.,1.,1.);

    float dist = length(viewPos - fragPosition);
    float depth = clamp((dist - fogStart) / (fogEnd - fogStart), 0.0, 1.0);

    finalColor = vec4(mix((finalColor).rgb, fogColor,depth) , 1.0f);
}