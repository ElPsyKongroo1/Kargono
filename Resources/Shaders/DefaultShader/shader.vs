#version 460 core
layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 vTextureCoord;

out vec2 fTextureCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
   gl_Position = projection * view * model * vec4(inPosition, 1.0);
   fTextureCoord = vTextureCoord;
}