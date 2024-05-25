#ifndef CHUNK_H
#define CHUNK_H

#include <vector>
#include "block.h"
#include "glad/glad.h"

class Chunk{

public:
  Chunk();
  ~Chunk();

  glm::vec2 chunkCenter;
  int Blocks[15][75][15];
  std::vector<Face> faces;
  std::vector<Face> waterFaces;
  std::vector<Face> foliageFaces;

  void Init();
  void Render();
  void RenderWater();
  void RenderFoliage();
  bool isInit = false;
  bool isKill = false;

private:
  void Enable();
  void BindVAOVBO();
  void BindWaterVAOVBO();
  void BindFoliageVAOVBO();
  void UnbindVAOVBO();

  unsigned int VAO = 0;
  unsigned int VBO = 0;
  unsigned int waterVAO = 0;
  unsigned int waterVBO = 0;
  unsigned int foliageVAO = 0;
  unsigned int foliageVBO = 0;
};

#endif // !CHUNK_H
