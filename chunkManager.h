#ifndef CHUNKMANAGER_H
#define CHUNKMANAGER_H

#include "chunk.h"
#include "chunkGenerator.h"
#include "glm/fwd.hpp"
#include "resourceManager.h"
#include "texture.h"
#include <memory>
#include <unordered_map>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

typedef std::unordered_map<glm::vec2, std::shared_ptr<Chunk>> ChunkMap;

enum ChunkSide{
  CHUNKLEFT,
  CHUNKRIGHT,
  CHUNKFORWARD,
  CHUNKBACKWARD
};

struct ChunkWithSide{
  glm::vec2 chunkCenter;
  glm::vec3 blockIndex;
  ChunkSide blockDirection;
};

class ChunkManager{
public:
  ChunkManager(Shader &shader_, Shader &waterShader_, Shader &foliageShader_, Texture2D &texture_);
  ~ChunkManager();
  void Init();
  void Update(glm::mat4 projection, glm::mat4 view, glm::vec3 camPos, glm::vec3 camFront, bool isGenBusy);
  void GenerateAroundCam(glm::vec2 camPos);
  bool isRendererBusy = false;
  void RequestCollisionCoords();
  std::shared_ptr<std::vector<glm::vec3>> collisionCoords;
  void SetBlockToBeBroken(glm::vec3 blockCoord);
  void SetBlockToBePlaced(glm::vec3 blockCoord, int blockID);

private:
  ChunkMap chunks;
  Shader shader;
  Shader waterShader;
  Shader foliageShader;
  Texture2D texture;
  glm::vec2 GetRelatedChunkCenter(glm::vec2 position);
  int seed;
  std::vector<glm::vec2> currentChunkCenters;
  std::vector<std::shared_ptr<Chunk>> newlyGeneratedChunks;
  std::vector<glm::vec2> chunkCentersForRendering;
  bool didGameRequestCollisionCoords = false;
  std::vector<glm::vec3> tempCollisionCoords;
  glm::vec3 blockToBeBroken;
  glm::vec2 chunkOfBlockToBeBroken;
  bool isBlockBreak = false;
  glm::vec3 blockToBePlaced;
  glm::vec2 chunkOfBlockToBePlaced;
  bool isBlockPlace = false;
  int blockToBePlacedID;
  //[0] reserved for X, [1] reserved for Z
  ChunkWithSide faceAddedUpdatingChunks[2];
  ChunkWithSide faceRemovedUpdatingChunks[2];

  void CheckAndUpdateIfFaceAdded(glm::vec2 chunkCenter);
  void CheckAndUpdateIfFaceRemoved(glm::vec2 chunkCenter);
  void CheckAndUpdateIfBlockPlaced(glm::vec2 chunkCenter);
  void CheckAndUpdateIfBlockBroken(glm::vec2 chunkCenter);
};

#endif // !CHUNKMANAGER_H
