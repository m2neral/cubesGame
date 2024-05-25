#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture1;

void main(){
  vec4 texel = texture(texture1, TexCoords);
  if(texel.a < 0.5){
    discard;
  }
  vec3 ambient = vec3(0.75, 0.75, 0.75) * vec3(texel);
  vec3 lightDir = normalize(vec3(0.0, 1.0, 0.0));
  float diff = dot(normalize(Normal), lightDir);
  vec3 diffuse = vec3(0.3, 0.3, 0.3) * diff * vec3(texel);
  FragColor = vec4(ambient + diffuse, texel.w);
}
