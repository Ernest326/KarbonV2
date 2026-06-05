//Modern PBR shader based on the Disney BRDF model

#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;
uniform vec3 viewPos;
uniform float materialShininess = 32.0;

//  INPUT Definitions ----------------------
struct GPUMaterial {
    vec4 albedoColor;
    vec4 emissiveColor;
    float roughness;
    float metallic;
    float hasAlbedoMap;
    float hasNormalMap;
    float hasRoughnessMap;
    float hasMetallicMap;
    float hasEmissiveMap;
    float hasAOMap;
};
layout(std140) uniform Materials {
    GPUMaterial materials[256];
};
uniform int materialIndex;
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;
uniform sampler2D emissiveMap;

//IBL stuff
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;
uniform int u_HasIBL;  // 0 = no cubemap, 1 = IBL active

struct PointLight {
    vec3  position;    // bytes  0-11
    float radius;      // bytes 12-15
    vec3  color;       // bytes 16-27
    float intensity;   // bytes 28-31
    float falloff;     // bytes 32-35
    float pad1;        // bytes 36-39  (individual floats — NOT float[3]!
    float pad2;        // bytes 40-43   float[N] in std140 has stride 16,
    float pad3;        // bytes 44-47   blowing struct size to 96 bytes)
}; // 48 bytes — matches C++ GPUPointLight

struct DirectionalLight {
    vec3  direction;   // bytes  0-11
    float intensity;   // bytes 12-15  (packed into vec3 tail — matches C++)
    vec3  color;       // bytes 16-27
    float padding;     // bytes 28-31
};

struct SpotLight {
    vec3  position;     // bytes  0-11
    float cutOff;       // bytes 12-15
    vec3  direction;    // bytes 16-27
    float outerCutOff;  // bytes 28-31
    vec3  color;        // bytes 32-43
    float intensity;    // bytes 44-47
    float falloff;      // bytes 48-51
    float pad1;         // bytes 52-55  (individual floats, not float[3])
    float pad2;         // bytes 56-59
    float pad3;         // bytes 60-63
}; // 64 bytes — matches C++ GPUSpotLight

layout(std140) uniform Lights {
    int u_PointLightCount;
    int u_DirectionalLightCount;
    int u_SpotLightCount;
    int padding;

    PointLight u_PointLights[64];
    DirectionalLight u_DirectionalLights[8];
    SpotLight u_SpotLights[16];
};

// PBR Functions ----------------------

const float PI = 3.14159265;

float DistributionGGX(vec3 N, vec3 H, float a) {
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float k) {
    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k; 
    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float k) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, k);
    float ggx1 = GeometrySchlickGGX(NdotL, k);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

//Helper functions -----------------------
vec3 getAlbedo(int materialIndex) {
    GPUMaterial mat = materials[materialIndex];
    if (mat.hasAlbedoMap == 1.0) {
        return texture(albedoMap, TexCoord).rgb;
    }
    return mat.albedoColor.rgb;
}

vec3 getNormal(int materialIndex) {
    GPUMaterial mat = materials[materialIndex];
    if (mat.hasNormalMap == 1.0) {
        return texture(normalMap, TexCoord).rgb;
    }
    return normalize(Normal);
}

float getMetallic(int materialIndex) {
    GPUMaterial mat = materials[materialIndex];
    if (mat.hasMetallicMap == 1.0) {
        return texture(metallicMap, TexCoord).r;
    }
    return mat.metallic;
}

float getRoughness(int materialIndex) {
    GPUMaterial mat = materials[materialIndex];
    if (mat.hasRoughnessMap == 1.0) {
        return texture(roughnessMap, TexCoord).r;
    }
    return mat.roughness;
}

float getAO(int materialIndex) {
    GPUMaterial mat = materials[materialIndex];
    if (mat.hasAOMap == 1.0) {
        return texture(aoMap, TexCoord).r;
    }
    return 1.0;
}

vec3 getEmissive(int materialIndex) {
    GPUMaterial mat = materials[materialIndex];
    if (mat.hasEmissiveMap == 1.0) {
        return texture(emissiveMap, TexCoord).rgb;
    }
    return mat.emissiveColor.rgb;
}

// -- Lighting calculations ----------------------

float attenuation(float distance, float radius, float falloff) {
    if (radius <= 0.0) return 0.0;
    float t = distance/radius;
    return pow(1.0-clamp(t,0.0,1.0), falloff);
}

float DirectK(float roughness) {
    float r = roughness + 1.0;
    return (r * r) / 8.0;
}

vec3 computePointLight(PointLight light, vec3 N, vec3 fragPos, vec3 V,
                       vec3 albedo, float metallic, float roughness, float ao,
                       vec3 F0, float NdotV) {
    vec3 lightDir = light.position - fragPos;
    float distance = length(lightDir);
    lightDir = normalize(lightDir);

    if (distance > light.radius) return vec3(0.0);

    float NdotL = max(dot(N, lightDir), 0.0);
    if (NdotL <= 0.0) return vec3(0.0);

    vec3 halfway = normalize(lightDir + V);

    float D = DistributionGGX(N, halfway, roughness);
    float k = DirectK(roughness);
    float G = GeometrySmith(N, V, lightDir, k);
    vec3 F = fresnelSchlick(max(dot(halfway, V), 0.0), F0);

    vec3 nominator = D * G * F;
    float denominator = 4.0 * NdotV * NdotL + 0.001;
    vec3 specular = nominator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    vec3 radiance = light.color * light.intensity * attenuation(distance, light.radius, light.falloff);
    return (kD * albedo / PI + specular) * radiance * NdotL * ao;
}

vec3 computeDirectionalLight(DirectionalLight light, vec3 N, vec3 V,
                             vec3 albedo, float metallic, float roughness, float ao,
                             vec3 F0, float NdotV) {


    vec3 lightDir = normalize(-light.direction);

    float NdotL = max(dot(N, lightDir), 0.0);
    if (NdotL <= 0.0) return vec3(0.0);

    vec3 halfway = normalize(lightDir + V);

    float D = DistributionGGX(N, halfway, roughness);
    float k = DirectK(roughness);
    float G = GeometrySmith(N, V, lightDir, k);
    vec3 F = fresnelSchlick(max(dot(halfway, V), 0.0), F0);

    vec3 nominator = D * G * F;
    float denominator = 4.0 * NdotV * NdotL + 0.001;
    vec3 specular = nominator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    vec3 radiance = light.color * light.intensity;
    return (kD * albedo / PI + specular) * radiance * NdotL * ao;
}

vec3 computeSpotLight(SpotLight light, vec3 N, vec3 fragPos, vec3 V,
                        vec3 albedo, float metallic, float roughness, float ao,
                        vec3 F0, float NdotV) {

    vec3 lightDir = light.position - fragPos;
    float distance = length(lightDir);
    lightDir = normalize(lightDir);

    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float spotIntensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    if (spotIntensity <= 0.0) return vec3(0.0);

    float NdotL = max(dot(N, lightDir), 0.0);
    if (NdotL <= 0.0) return vec3(0.0);

    vec3 halfway = normalize(lightDir + V);

    float D = DistributionGGX(N, halfway, roughness);
    float k = DirectK(roughness);
    float G = GeometrySmith(N, V, lightDir, k);
    vec3 F = fresnelSchlick(max(dot(halfway, V), 0.0), F0);

    vec3 nominator = D * G * F;
    float denominator = 4.0 * NdotV * NdotL + 0.001;
    vec3 specular = nominator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    vec3 radiance = light.color * light.intensity * spotIntensity;
    return (kD * albedo / PI + specular) * radiance * NdotL * ao;
}

vec3 computeIBL(vec3 N, vec3 V, vec3 albedo, float metallic, float roughness, float ao) {
    if (u_HasIBL == 0) return vec3(0.03) * albedo * ao;  // flat fallback
    
    float NdotV = max(dot(N, V), 0.0);
    vec3 R = reflect(-V, N);
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    vec3 F = fresnelSchlickRoughness(NdotV, F0, roughness);
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;
    
    // Diffuse
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo;
    
    // Specular
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(brdfLUT, vec2(NdotV, roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
    
    return (kD * diffuse + specular) * ao;
}

// Extra functions

vec3 ACESFilm(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

vec3 UnrealTonemap(vec3 x) {
    return x / (x + 0.155) * 1.019;
}


// -- Main function ----------------------

void main() {

    // --- Material sampling ---
    vec3 albedo     = getAlbedo(materialIndex);
    vec3 emissive   = getEmissive(materialIndex);
    float metallic  = getMetallic(materialIndex);
    float roughness = getRoughness(materialIndex);
    float ao        = getAO(materialIndex);

    // --- Normal ---
    vec3 N = getNormal(materialIndex);
    vec3 V = normalize(viewPos - FragPos);
    float NdotV = max(dot(N, V), 0.0);

    // --- Fresnel base ---
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    // --- Lighting accumulation ---
    vec3 Lo = vec3(0.0);

    for (int i = 0; i < u_PointLightCount; ++i) {
        Lo += computePointLight(u_PointLights[i], N, FragPos, V, albedo, metallic, roughness, ao, F0, NdotV);
    }
    for (int i = 0; i < u_DirectionalLightCount; ++i) {
        Lo += computeDirectionalLight(u_DirectionalLights[i], N, V, albedo, metallic, roughness, ao, F0, NdotV);
    }
    for (int i = 0; i < u_SpotLightCount; ++i) {
        Lo += computeSpotLight(u_SpotLights[i], N, FragPos, V, albedo, metallic, roughness, ao, F0, NdotV);
    }

    // --- Ambient (IBL placeholder or flat fallback) ---
    vec3 ambient = computeIBL(N, V, albedo, metallic, roughness, ao);

    // --- Compose ---
    vec3 color = ambient + Lo + emissive;

    // --- Tonemapping (Reinhard is muddy, use ACES or Unreal) ---
    color = ACESFilm(color);

    // --- Gamma correction ---
    color = pow(max(color, vec3(0.0)), vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}