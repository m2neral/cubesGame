#include "shader.h"
#include <iostream>

Shader &Shader::Use(){
  glUseProgram(this->ID);
  return *this;
}

void Shader::Compile(const char* vertPath, const char* fragPath, const char*, const char* geoPath){
  unsigned int vertSh, fragSh, geoSh;

  vertSh = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertSh, 1, &vertPath, NULL);
  glCompileShader(vertSh);
  checkCompileErrors(vertSh, "VERTEX");
  
  fragSh = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragSh, 1, &fragPath, NULL);
  glCompileShader(fragSh);
  checkCompileErrors(fragSh, "FRAGMENT");

  if(geoPath != nullptr){
    geoSh = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(geoSh, 1, &geoPath, NULL);
    glCompileShader(geoSh);
    checkCompileErrors(geoSh, "GEOMETRY");
  }

  this->ID = glCreateProgram();
  glAttachShader(this->ID, vertSh);
  glAttachShader(this->ID, fragSh);
  if(geoPath != nullptr){glAttachShader(this->ID, geoSh);}
  glLinkProgram(this->ID);
  checkCompileErrors(this->ID, "PROGRAM");
  
  glDeleteShader(vertSh);
  glDeleteShader(fragSh);
  if(geoPath != nullptr){glDeleteShader(geoSh);}
}

void Shader::SetFloat(const char* name, float value, bool use){
  if(use){this->Use();}
  glUniform1f(glGetUniformLocation(this->ID, name), value);
}
void Shader::SetInt(const char* name, int value, bool use){
  if(use){this->Use();}
  glUniform1i(glGetUniformLocation(this->ID, name), value);
}
void Shader::SetVec2(const char* name, float x, float y, bool use){
  if(use){this->Use();}
  glUniform2f(glGetUniformLocation(this->ID, name), x, y);
}
void Shader::SetVec2(const char* name, const glm::vec2 &value, bool use){
  if(use){this->Use();}
  glUniform2f(glGetUniformLocation(this->ID, name), value.x, value.y);
}
void Shader::SetVec3(const char* name, float x, float y, float z, bool use){
  if(use){this->Use();}
  glUniform3f(glGetUniformLocation(this->ID, name), x, y, z);
}
void Shader::SetVec3(const char* name, const glm::vec3 &value, bool use){
  if(use){this->Use();}
  glUniform3f(glGetUniformLocation(this->ID, name), value.x, value.y, value.z);
}
void Shader::SetVec4(const char* name, float x, float y, float z, float w, bool use){
  if(use){this->Use();}
  glUniform4f(glGetUniformLocation(this->ID, name), x, y, z, w);
}
void Shader::SetVec4(const char* name, const glm::vec4 &value, bool use){
  if(use){this->Use();}
  glUniform4f(glGetUniformLocation(this->ID, name), value.x, value.y, value.z, value.w);
}
void Shader::SetMat4(const char* name, const glm::mat4 &matrix, bool use){
  if(use){this->Use();}
  glUniformMatrix4fv(glGetUniformLocation(this->ID, name), 1, false, glm::value_ptr(matrix));
}

void Shader::checkCompileErrors(unsigned int object, std::string type){
  int success;
  char infoLog[1024];
  if(type != "PROGRAM"){
    glGetShaderiv(object, GL_COMPILE_STATUS, &success);
    if(!success){
      glGetShaderInfoLog(object, 1024, NULL, infoLog);
      std::cout << "Error: Shader failed to compile. Type: " << type << "\n" << infoLog << "\n ========================" << std::endl;
    }
  }else{
    glGetProgramiv(object, GL_LINK_STATUS, &success);
    if(!success){
      glGetProgramInfoLog(object, 1024, NULL, infoLog);
      std::cout << "Error: Shader failed to link. Type: " << type << "\n" << infoLog << "\n ========================" << std::endl;
    }
  }
}
