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
    finalColor = texelColor*(ambient/10.0)*tint;

    vec4 lightCol = vec4(0,0,0,0);
    

    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (lights[i].enabled == 1)
        {
            //vec3 light = vec3(0.0);

            if (lights[i].type == LIGHT_DIRECTIONAL)
            {
                //light = -normalize(lights[i].target - lights[i].position);
            }

            if (lights[i].type == LIGHT_POINT)
            {
                //light = normalize(lights[i].position - fragPosition);
                lightCol += mix(lights[i].color, noLightColor, min(length(lights[i].position - fragPosition)/3,1));
            }

            //float NdotL = max(dot(normal, light), 0.0);
            //lightDot += lights[i].color.rgb*NdotL;

            //float specCo = 0.0;
            //if (NdotL > 0.0) specCo = pow(max(0.0, dot(viewD, reflect(-(light), normal))), 16.0); // 16 refers to shine
            //specular += specCo;
        }
    }

    finalColor += texelColor*lightCol;

    //finalColor = (texelColor*((tint + vec4(specular, 1.0))*vec4(lightDot, 1.0)));

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