#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 u_MVP;
uniform float u_Scale;

void main() {
    gl_Position = u_MVP * vec4(aPos * u_Scale, 1.0);
}