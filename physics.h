#ifndef PHYSICS_H
#define PHYSICS_H

#include "glm/glm.hpp"
#include <memory>
#include <vector>
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/scalar_constants.hpp"
#include "glm/trigonometric.hpp"
#include <iostream>
#include <ostream>
#include <algorithm>
#include <cmath>
#include <ostream>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"
#include "glm/gtx/string_cast.hpp"

enum SixDirections{
  AABB_RIGHT,
  AABB_LEFT,
  AABB_UP,
  AABB_DOWN,
  AABB_FORWARD,
  AABB_BACKWARD
};

class Physics{

public:
  Physics();
  ~Physics();
  void ProcessWalk();
  glm::vec3 Update(glm::vec3 front_, glm::vec3 fakeFront_, float deltaTime, std::shared_ptr<std::vector<glm::vec3>> collisionCoords_);

  bool walkForward = false;
  bool walkBackward = false;
  bool walkLeft = false;
  bool walkRight = false;
  bool shouldJump = false;
  bool initial = true;
  bool showIndicator = false;
  glm::vec3 indicatorPosition;
  glm::vec3 indicatorDirection;

  glm::vec3 Lerp(glm::vec3 targetVelocity, glm::vec3 currentVelocity, float weight);

  std::vector<glm::vec3> collisionCoords;

private:
  glm::vec3 fakeFront;
  glm::vec3 front;
  glm::vec3 moveDirection;
  float gravity = 0.035f;
  float moveSpeed = 0.2f;
  float jumpForce = 0.275f;
  glm::vec3 velocity = glm::vec3(0.0f, 0.0f, 0.0f);
  bool CollisionCheckAABB(glm::vec3 blockCenter, glm::vec3 playerPosition);
  SixDirections GetCollisionDirection(glm::vec3 target);
  void Jump();
  void Landed();
  bool isGrounded;
  bool wasGrounded;
  bool isFalling;
  bool isJumping;
  glm::vec3 playerPos = glm::vec3(0.0f, 50.0f, 0.0f);
  glm::vec3 lastKnownPosition = glm::vec3(0.0f, 50.0f, 0.0f);
  
};

#endif // !PHYSICS_H

