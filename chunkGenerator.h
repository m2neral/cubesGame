#ifndef CHUNKGENERATOR_H
#define CHUNKGENERATOR_H

#include "chunk.h"
#include <ctime>
#include <memory>

enum BlockType{
  BLOCK,
  WATER,
  FLOWER,
  LEAVES
};

class ChunkGenerator{
public:
  ChunkGenerator();
  ~ChunkGenerator();
  std::shared_ptr<Chunk> GenerateChunk(glm::vec2 chunkCenter, int seed);
  void GenerateFaces(glm::vec2 chunkCenter, std::shared_ptr<Chunk>, int seed);
  static bool CheckBlockType(BlockType thisBlockType, int otherBlockID);

private:
  int* PerlinChunk(int X, int Z, int seed);
  float perlin(float x, float y, int seed);
  float dotGridGradient(int ix, int iy, float x, float y, int seed);
  float interpolate(float a0, float a1, float w);
  glm::vec2 randomGradient(int ix, int iy, int seed);
};

#endif // !CHUNKGENERATOR_H
