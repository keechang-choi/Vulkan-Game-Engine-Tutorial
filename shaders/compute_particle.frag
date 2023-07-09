#version 450

layout (location = 0) in vec3 fragColor;

layout (location = 0) out vec4 outColor;

void main() {
    vec2 coord = gl_PointCoord - vec2(0.5);
    // float alpha = max(0.25 - dot(coord, coord), 0.0) * 4.0;
    float alpha = float(0.25 > dot(coord, coord));
    
    outColor = vec4(fragColor, alpha);
}