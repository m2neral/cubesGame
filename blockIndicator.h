#ifndef BLOCKINDICATOR_H
#define BLOCKINDICATOR_H

#include "glad/glad.h"
#include "block.h"
#include "glm/fwd.hpp"
#include <vector>

class BlockIndicator {
public:
  BlockIndicator();
  ~BlockIndicator();
  void Init();
  void Render();

private:
  void Enable();
  void BindVAOVBO();
  void UnbindVAOVBO();
  
  std::vector<Face> faces;

  unsigned int VAO = 0;
  unsigned int VBO = 0;
  
};


#endif // !BLOCKINDICATOR_H
