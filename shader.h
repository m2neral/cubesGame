#ifndef SHADER_H
#define SHADER_H
#include "glad/glad.h"
#include "glm/fwd.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <string>

class Shader{
public:
  unsigned int ID;
  Shader(){}
  Shader &Use();
  void Compile(const char* vertPath, const char* fragPath, const char*, const char* geoPath = nullptr);
  void SetFloat(const char* name, float value, bool use = false);
  void SetInt(const char* name, int value, bool use = false);
  void SetVec2(const char* name, float x, float y, bool use = false);
  void SetVec2(const char* name, const glm::vec2 &value, bool use = false);
  void SetVec3(const char* name, float x, float y, float z, bool use = false);
  void SetVec3(const char* name, const glm::vec3 &value, bool use = false);
  void SetVec4(const char* name, float x, float y, float z, float w, bool use = false);
  void SetVec4(const char* name, const glm::vec4 &value, bool use = false);
  void SetMat4(const char* name, const glm::mat4 &value, bool use = false);
private:
  void checkCompileErrors(unsigned int object, std::string type);
};

#endif // !SHADER_H
