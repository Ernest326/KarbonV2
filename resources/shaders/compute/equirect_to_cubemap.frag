#version 330 core
in vec3 localPos;
out vec4 FragColor;

uniform sampler2D equirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183);

vec2 sampleSphericalMap(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main() {
    vec2 uv = sampleSphericalMap(normalize(localPos));
    vec3 color = texture(equirectangularMap, uv).rgb;
    // FIX: clamp radiance below half-float infinity. HDRs store the sun as
    // values > 65504, which become +inf when written to an RGB16F target.
    // One inf texel turns the ENTIRE irradiance map and prefilter mip chain
    // to inf during convolution, and inf becomes NaN (rendered as black) in
    // the ACES tonemapper: inf/inf. 500 keeps the sun visually saturated
    // while bounding the convolution integrals.
    color = min(color, vec3(500.0));
    FragColor = vec4(color, 1.0);
}
