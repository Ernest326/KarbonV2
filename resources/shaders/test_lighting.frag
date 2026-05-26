#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

struct PointLight {
    vec3 position;
    float radius;
    vec3 color;
    float intensity;
    float falloff;
};

uniform vec3 viewPos;
uniform sampler2D diffuseTexture;
uniform float materialShininess = 32.0;

layout(std140) uniform Lights {
    int u_PointLightCount;
    PointLight u_PointLights[64];
};

float attenuation(float distance, float radius, float falloff) {
    if (radius <= 0.0) return 0.0;
    float t = distance/radius;
    return pow(1.0-clamp(t,0.0,1.0), falloff);
};

//Simple Blinn-Phong lighting model
vec3 computePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo) {
    vec3 lightDir = light.position - fragPos;
    float distance = length(lightDir);

    if (distance > light.radius) return vec3(0.0);

    vec3 halfway = normalize(lightDir + viewDir);

    float diff = max(dot(normal, lightDir), 0.0);
    float spec = pow(max(dot(normal, halfway), 0.0), materialShininess);

    float attenuation = attenuation(distance, light.radius, light.falloff);
    
    vec3 diffuse = light.color * light.intensity * diff * albedo * attenuation;
    vec3 specular = light.color * light.intensity * spec * attenuation * 0.5;

    return diffuse + specular;
}

void main() {
    vec3 albedo = texture(diffuseTexture, TexCoord).rgb;
    vec3 normal = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = albedo*0.1;

    for (int i=0; i<u_PointLightCount; i++) {
        result += computePointLight(u_PointLights[i], normal, FragPos, viewDir, albedo);
    }
    result = result / (result + vec3(1.0));
    result = pow(result, vec3(1.0/2.2));
    FragColor = vec4(result, 1.0);
}

