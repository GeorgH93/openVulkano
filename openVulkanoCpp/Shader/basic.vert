#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 biTangent;
layout(location = 4) in vec3 textureCoordinates;
layout(location = 5) in vec4 color;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform NodeData
{
	mat4 world;
} node;

layout(std140, push_constant) uniform CameraData {
    mat4 viewProjection;
} cam;

void main()
{
	vec3 light = normalize(vec3(1));
	vec4 worldPos = node.world * vec4(position, 1.0);
    vec3 worldNormal = normalize(transpose(inverse(mat3(node.world))) * normal);
    float brightness = max(0.0, dot(worldNormal, light));
    outColor = vec4(clamp(color.rgb * (0.5 + brightness / 2), 0, 1), 1);
	gl_Position = normalize(cam.viewProjection *  worldPos);
}