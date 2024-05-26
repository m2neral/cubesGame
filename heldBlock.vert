#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;
layout(location = 2) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;
out vec3 lightPos;

void main(){
  FragPos = vec3(model * vec4(aPos, 1.0));
  Normal = mat3(transpose(inverse(model))) * aNormal;
  lightPos = vec3( view * vec4(0.0f, 1.0f, 0.0f, 0.0f));
  TexCoords = aTexCoords;
  gl_Position = projection * vec4(FragPos, 1.0);
}
