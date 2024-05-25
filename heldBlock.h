#ifndef HELDBLOCK_H
#define HELDBLOCK_H

#include "glad/glad.h"
#include "block.h"
#include "glm/fwd.hpp"

class HeldBlock {
public:
  HeldBlock();
  ~HeldBlock();
  void Init();
  void Render();
  std::vector<Face> faces;

private:
  void Enable();
  void BindVAOVBO();
  void UnbindVAOVBO();
  

  unsigned int VAO = 0;
  unsigned int VBO = 0;
  
};


#endif // !HELDBLOCK_H
