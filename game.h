#ifndef GAME_H
#define GAME_H

#include "camera.h"
#include "chunkManager.h"
#include "GLFW/glfw3.h"
#include "physics.h"
#include "glm/ext/matrix_clip_space.hpp"
#include <memory>
#include "blockIndicator.h"

class Game{
public:
  bool keys[1024];
  unsigned int width, height;
  Game(unsigned int width_, unsigned int height_);
  ~Game();
  void Init();
  void ProcessInput(float deltaTime);
  void PhysicsUpdate(float deltaTime);
  void Update(float deltaTime);
  void Render();
  void UpdateCamera(float xOffset, float yOffset);
  void Clear();
  static void ChunkGen();
  void AsyncChunkGen();
  glm::vec3 physicsPosition = glm::vec3(0.0f, 0.0f, 0.0f);
  void PlaceBlock();
  void BreakBlock();
private:
  Shader indicatorShader;
  Shader heldShader;
  void DrawIndicator();
  void DrawHeld();
  glm::vec3 blockIndicatorPos;
  glm::vec3 blockIndicatorDir;
};

#endif // !GAME_H
