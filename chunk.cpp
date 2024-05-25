#include "chunk.h"

Chunk::Chunk(){}

Chunk::~Chunk(){
  glDeleteVertexArrays(1, &VAO);
  VAO = 0;
  glDeleteBuffers(1, &VBO);
  VBO = 0;
  glDeleteVertexArrays(1, &waterVAO);
  waterVAO = 0;
  glDeleteBuffers(1, &waterVBO);
  waterVBO = 0;
  glDeleteVertexArrays(1, &foliageVAO);
  foliageVAO = 0;
  glDeleteBuffers(1, &foliageVBO);
  foliageVBO = 0;
  faces = std::vector<Face>();
  waterFaces = std::vector<Face>();
  foliageFaces = std::vector<Face>();
}

void Chunk::Init(){
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  BindVAOVBO();
  if(faces.empty() == false){
    glBufferData(GL_ARRAY_BUFFER, sizeof(Face) * faces.size() * 6, nullptr, GL_DYNAMIC_DRAW);
  }
  Enable();

  glGenVertexArrays(1, &waterVAO);
  glGenBuffers(1, &waterVBO);
  BindWaterVAOVBO();
  if(waterFaces.empty() == false){
    glBufferData(GL_ARRAY_BUFFER, sizeof(Face) * waterFaces.size(), nullptr, GL_DYNAMIC_DRAW);
  }
  Enable();

  glGenVertexArrays(1, &foliageVAO);
  glGenBuffers(1, &foliageVBO);
  BindFoliageVAOVBO();
  if(foliageFaces.empty() == false){
    glBufferData(GL_ARRAY_BUFFER, sizeof(Face) * foliageFaces.size(), nullptr, GL_DYNAMIC_DRAW);
  }
  Enable();
  
  UnbindVAOVBO();
}

void Chunk::Enable(){
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
}


void Chunk::Render(){
  BindVAOVBO();
  if(faces.empty() == false){
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Face) * faces.size(), &faces.front());
    glDrawArrays(GL_TRIANGLES, 0, faces.size() * 6);
  }
  UnbindVAOVBO();
}

void Chunk::RenderWater(){
  BindWaterVAOVBO();
  if(waterFaces.empty() == false){
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Face) * waterFaces.size(), &waterFaces.front());
    glDrawArrays(GL_TRIANGLES, 0, waterFaces.size() * 6);
  }
  UnbindVAOVBO();
}

void Chunk::RenderFoliage(){
  BindFoliageVAOVBO();
  if(foliageFaces.empty() == false){
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Face) * foliageFaces.size(), &foliageFaces.front());
    glDrawArrays(GL_TRIANGLES, 0, foliageFaces.size() * 6);
  }
  UnbindVAOVBO();
}

void Chunk::BindVAOVBO(){
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
}

void Chunk::UnbindVAOVBO(){
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Chunk::BindWaterVAOVBO(){
  glBindVertexArray(waterVAO);
  glBindBuffer(GL_ARRAY_BUFFER, waterVBO);
}

void Chunk::BindFoliageVAOVBO(){
  glBindVertexArray(foliageVAO);
  glBindBuffer(GL_ARRAY_BUFFER, foliageVBO);
}
