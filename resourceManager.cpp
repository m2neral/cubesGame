#include "resourceManager.h"
#include <fstream>
#include <iostream>
#include <sstream>

std::map<std::string, Texture2D> ResourceManager::Textures;
std::map<std::string, Shader> ResourceManager::Shaders;

Shader ResourceManager::LoadShader(const char* vertFile, const char* fragFile, const char* geoFile, std::string name){
  Shaders[name] = loadShaderFromFile(vertFile, fragFile);
  return Shaders[name];
}

Shader& ResourceManager::GetShader(std::string name){
  return Shaders[name];
}

Texture2D ResourceManager::LoadTexture(const char* file, bool alpha, std::string name){
  Textures[name] = loadTextureFromFile(file, alpha);
  return Textures[name];
}

Texture2D& ResourceManager::GetTexture(std::string name){
  return Textures[name];
}

void ResourceManager::Clear(){
  for(auto iter : Shaders){glDeleteProgram(iter.second.ID);}
  for(auto iter : Textures){glDeleteTextures(1, &iter.second.ID);}
}

Shader ResourceManager::loadShaderFromFile(const char* vertFile, const char* fragFile, const char* geoFile){
  std::string vertCode;
  std::string fragCode;
  std::string geoCode;
  try{
    std::ifstream vertShaderFile(vertFile);
    std::ifstream fragShaderFile(fragFile);
    std::stringstream vertShStream, fragShStream;
    vertShStream << vertShaderFile.rdbuf();
    fragShStream << fragShaderFile.rdbuf();
    vertShaderFile.close();
    fragShaderFile.close();
    vertCode = vertShStream.str();
    fragCode = fragShStream.str();
    if(geoFile != nullptr){
      std::ifstream geoShaderFile(geoFile);
      std::stringstream geoShStream;
      geoShStream << geoShaderFile.rdbuf();
      geoShaderFile.close();
      geoCode = geoShStream.str();
    }
  }catch(std::exception e){
    std::cout << "Error: Failed to Read Shader Files" << std::endl;
  }
  const char* vertShaderCode = vertCode.c_str();
  const char* fragShaderCode = fragCode.c_str();
  const char* geoShaderCode = geoCode.c_str();
  Shader shader;
  shader.Compile(vertShaderCode, fragShaderCode, geoFile != nullptr ? geoShaderCode : nullptr);
  return shader;
}

Texture2D ResourceManager::loadTextureFromFile(const char* file, bool alpha){
  Texture2D texture;
  if(alpha){
    texture.internalFormat = GL_RGBA;
    texture.imageFormat = GL_RGBA;
  }
  int width, height, nrChannels;
  stbi_set_flip_vertically_on_load(true);
  unsigned char* data = stbi_load(file, &width, &height, &nrChannels, 0);
  texture.Generate(width, height, data);
  stbi_image_free(data);
  return texture;
}
