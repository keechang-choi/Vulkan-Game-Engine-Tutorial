#version 450

const vec2 OFFSETS[6] = vec2[](
  vec2(-1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, -1.0),
  vec2(1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, 1.0)
);

layout (location = 0) out vec2 fragOffset;

struct PointLight{
  vec4 position; // ignore w
  vec4 color; // w as intensity
};

layout (set = 0, binding = 0) uniform GlobalUbo{
    mat4 projection;
    mat4 view;
    mat4 invView;
    vec4 ambientLightColor; // w as intensity
    PointLight pointLights[10];
    int numLights;
} ubo;

layout (push_constant) uniform Push{
  vec4 position;
  vec4 color;
  float radius;
} push;

void main(){
    fragOffset = push.radius * OFFSETS[gl_VertexIndex];
    vec3 cameraRightWorld = {ubo.view[0][0], ubo.view[1][0], ubo.view[2][0]};
    vec3 cameraUpWorld = {ubo.view[0][1], ubo.view[1][1], ubo.view[2][1]};

    // vec3 positionWorld = push.position.xyz
    //  + push.radius * fragOffset.x * cameraRightWorld
    //  + push.radius * fragOffset.y * cameraUpWorld;
    vec4 lightInCameraSpace = ubo.view * vec4(push.position.xyz, 1.0);
    vec4 positionInCameraSpace = lightInCameraSpace + vec4(fragOffset, 0.0, 0.0);

    gl_Position = ubo.projection * positionInCameraSpace;
    
}