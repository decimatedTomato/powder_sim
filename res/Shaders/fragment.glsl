#version 330 core

in vec4 gl_FragCoord;
out vec4 fragColor;

uniform vec2 u_resolution;
uniform float u_time;
uniform sampler2D u_texture;

void main() {
    fragColor = texture2D(u_texture, v_texCoords) * v_color;;
};