#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;
in vec3 lightPos;

uniform sampler2D texture1;

void main(){
  vec3 ambient = vec3(0.6, 0.6, 0.6) * vec3(texture(texture1, TexCoords));
  // vec3 lightDir = normalize(vec3(1000.0, 1000.0, 1000.0) - FragPos);
  vec3 lightDir = normalize(lightPos);
  float diff = dot(normalize(Normal), lightDir);
  vec3 diffuse = vec3(0.3, 0.3, 0.3) * diff * vec3(texture(texture1, TexCoords));
  FragColor = vec4(ambient + diffuse, 1.0);
  // FragColor = texture(texture1, TexCoords);
}
