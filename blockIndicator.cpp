#include "blockIndicator.h"

BlockIndicator::BlockIndicator(){

}
BlockIndicator::~BlockIndicator(){
  glDeleteVertexArrays(1, &VAO);
  VAO = 0;
  glDeleteBuffers(1, &VBO);
  VBO = 0;
  faces.clear();
}

void BlockIndicator::Init(){
  Block coords = Block(4, glm::vec3(0.0f, 0.0f, 0.0f));
  faces.push_back(coords.faceTop);
  faces.push_back(coords.faceBottom);
  faces.push_back(coords.faceLeft);
  faces.push_back(coords.faceRight);
  faces.push_back(coords.faceBehind);
  faces.push_back(coords.faceFront);
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  BindVAOVBO();
  glBufferData(GL_ARRAY_BUFFER, sizeof(Face) * faces.size(), nullptr, GL_DYNAMIC_DRAW);
  Enable();
  UnbindVAOVBO();
}

void BlockIndicator::Render(){
  BindVAOVBO();
  if(faces.empty() == false){
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Face) * faces.size(), &faces.front());
    glDrawArrays(GL_TRIANGLES, 0, faces.size() * 6);
  }
  UnbindVAOVBO();
}

void BlockIndicator::Enable(){
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
}

void BlockIndicator::BindVAOVBO(){
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
}

void BlockIndicator::UnbindVAOVBO(){
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}
