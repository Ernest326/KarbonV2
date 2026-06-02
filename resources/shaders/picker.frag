#version 330 core
uniform vec4 u_ID;
out vec4 FragColor;
void main() {
    FragColor = u_ID;
}