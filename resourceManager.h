#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <map>
#include <string>
#include "glad/glad.h"
#include "texture.h"
#include "shader.h"
#include "stb_image.h"

class ResourceManager{
public:
  static std::map<std::string, Shader> Shaders;
  static std::map<std::string, Texture2D> Textures;
  static Shader LoadShader(const char* vertFile, const char* fragFile, const char* geoFile, std::string name);
  static Shader& GetShader(std::string name);
  static Texture2D LoadTexture(const char* file, bool alpha, std::string name);
  static Texture2D& GetTexture(std::string name);
  static void Clear();
private:
  ResourceManager(){}
  static Shader loadShaderFromFile(const char* vertFile, const char* fragFile, const char* geoFile = nullptr);
  static Texture2D loadTextureFromFile(const char* file, bool alpha);
};

#endif // !RESOURCEMANAGER_H
