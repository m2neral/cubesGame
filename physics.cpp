#include "physics.h"

Physics::Physics(){}

Physics::~Physics(){}

glm::vec3 sixDirections[] = {
  glm::vec3(1.0f, 0.0f, 0.0f), //right
  glm::vec3(-1.0f, 0.0f, 0.0f), //left
  glm::vec3(0.0f, 1.0f, 0.0f), //up
  glm::vec3(0.0f, -1.0f, 0.0f), //down
  glm::vec3(0.0f, 0.0f, 1.0f), //forward
  glm::vec3(0.0f, 0.0f, -1.0f) //backward
};

void Physics::ProcessWalk(){
  //The walk direction is set based on keyboard input.
  //It is then rotated by the "fakeFront" vector, which is explained below. 
  float dirBackwardForward = 0.0f;
  float dirLeftRight = 0.0f;
  if((walkBackward && walkForward) || (!walkBackward && !walkForward)){dirBackwardForward = 0.0f;}
  else if(walkForward){dirBackwardForward = -1.0f;}
  else if(walkBackward){dirBackwardForward = 1.0f;}

  if((walkLeft && walkRight) || (!walkLeft && !walkRight)){dirLeftRight = 0.0f;}
  else if(walkLeft){dirLeftRight = -1.0f;}
  else if(walkRight){dirLeftRight = 1.0f;}

  glm::vec3 tempDirection = glm::vec3(dirLeftRight, 0.0f, dirBackwardForward);
  if(dirBackwardForward != 0.0f || dirLeftRight != 0.0f){
    glm::mat4 moveMatrix = glm::mat4(1.0f);
    moveMatrix = glm::rotate(moveMatrix, glm::radians(((glm::atan(fakeFront.x, fakeFront.z) / glm::pi<float>() ) + 1.0f) * 180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    tempDirection = moveMatrix * glm::vec4(tempDirection, 0.0f);
    tempDirection = glm::normalize(tempDirection);
    moveDirection = glm::vec3(tempDirection.x, 0.0f, tempDirection.z);
  }else{
    moveDirection = glm::vec3(0.0f, 0.0f, 0.0f);
  }
}

SixDirections Physics::GetCollisionDirection(glm::vec3 target){
  float max = 0.0f;
  unsigned int best_match = -1;
  for(unsigned int i = 0; i < 6; i++){
    float dot_product = glm::dot(glm::normalize(target), sixDirections[i]);
    if(dot_product > max){
      max = dot_product;
      best_match = i;
    }
  }
  return (SixDirections)best_match;
}

bool Physics::CollisionCheckAABB(glm::vec3 blockCenter, glm::vec3 playerPosition){
  float BlockAABB = 0.5f;
  float blockMinX = blockCenter.x - BlockAABB;
  float blockMaxX = blockCenter.x + BlockAABB;
  float blockMinY = blockCenter.y - BlockAABB;
  float blockMaxY = blockCenter.y + BlockAABB;
  float blockMinZ = blockCenter.z - BlockAABB;
  float blockMaxZ = blockCenter.z + BlockAABB;

  float playerMinX = playerPos.x - 0.3f;
  float playerMaxX = playerPos.x + 0.3f;
  float playerMinY = playerPos.y - 0.9f;
  float playerMaxY = playerPos.y + 0.9f;
  float playerMinZ = playerPos.z - 0.3f;
  float playerMaxZ = playerPos.z + 0.3f;

  return (blockMinX <= playerMaxX && blockMaxX >= playerMinX && blockMinY <= playerMaxY && blockMaxY >= playerMinY && blockMinZ <= playerMaxZ && blockMaxZ >= playerMinZ);
}

void Physics::Jump(){
  shouldJump = false;
  if(isJumping == false && isGrounded){
    isGrounded = false;
    isJumping = true;
    velocity.y = jumpForce;
  }
}

void Physics::Landed(){
  isJumping = false;
  isFalling = false;
  isGrounded = true;
  velocity.y = 0.0f;
}

//Front is where the camera is looking at, fakeFront is where the camera is looking at without the up and down directions.
//Front is used for the block indicator, fakeFront is used for movement direction.
glm::vec3 Physics::Update(glm::vec3 front_, glm::vec3 fakeFront_, float deltaTime, std::shared_ptr<std::vector<glm::vec3>> collisionCoords_){
  //Collision coordinates set by the chunkManager are used here.
  //This check adds the coordinates to the collisionCoords vector is they're within 5.0f of the player and are also not already in the vector.
  if(collisionCoords_->empty() == false){
    collisionCoords = std::vector<glm::vec3>();
    for(std::vector<glm::vec3>::iterator it = collisionCoords_->begin(); it != collisionCoords_->end(); it++){
      if(glm::distance(*it, playerPos) <= 5.0f){
        if(std::find(collisionCoords.begin(), collisionCoords.end(), *it) == collisionCoords.end()){
          collisionCoords.push_back(*it);
        }
      }
    }
  }
  front = front_;
  fakeFront = fakeFront_;

  //Gravity and velocity (except for interpolating) don't use deltaTime because physics already run at a constant 30fps.
  
  if(!isGrounded && !initial){velocity.y -= gravity;}

  if(shouldJump){Jump();}

  glm::vec3 speed = moveDirection * moveSpeed;
  velocity = Lerp(glm::vec3(speed.x, velocity.y, speed.z), velocity, 30.0f * deltaTime);
  playerPos += velocity;


  if(!isGrounded && velocity.y < 0){isFalling = true;}
  else{isFalling = false;}


  bool isTouchingSomething = false;
  for(auto &it : collisionCoords){
    if(CollisionCheckAABB(it, playerPos)){
      isTouchingSomething = true;
      SixDirections collisionDirection = GetCollisionDirection(it - playerPos);
      //Set player position based on AABB collisions. Will probably make this also affect velocity.
      if(collisionDirection == AABB_DOWN){
        float difference = (it.y + 0.5f) - (playerPos.y - 0.9f);
        playerPos.y += difference;
        lastKnownPosition = playerPos;
        if(isFalling){isGrounded = true;}
      }else if(collisionDirection == AABB_UP){
        float difference = (it.y - 0.5f) - (playerPos.y + 0.9f);
        playerPos.y += difference;
      }else if(collisionDirection == AABB_LEFT){
        float difference = (it.x + 0.5f) - (playerPos.x - 0.3f);
        playerPos.x += difference;
      }else if(collisionDirection == AABB_RIGHT){
        float difference = (it.x - 0.5f) - (playerPos.x + 0.3f);
        playerPos.x += difference;
      }else if(collisionDirection == AABB_BACKWARD){
        float difference = (it.z + 0.5f) - (playerPos.z - 0.3f);
        playerPos.z += difference;
      }else if(collisionDirection == AABB_FORWARD){
        float difference = (it.z - 0.5f) - (playerPos.z + 0.3f);
        playerPos.z += difference;
      }
    }
  }

  //This determines the block indicator position for breaking blocks.
  //It also roughly determines the direction for placing blocks.
  showIndicator = false;
  for(float i = 0.0f; i < 5.0f; i += 0.1f){
    glm::vec3 camPos = glm::vec3(playerPos.x, playerPos.y + 0.72f, playerPos.z);
    glm::vec3 rayPos = camPos + front * i;
    glm::vec3 rayPosRounded;
    rayPosRounded.x = round(rayPos.x);
    rayPosRounded.y = round(rayPos.y);
    rayPosRounded.z = round(rayPos.z);
    if(std::find(collisionCoords.begin(), collisionCoords.end(), rayPosRounded) != collisionCoords.end()){
      indicatorPosition = rayPosRounded;
      indicatorDirection = sixDirections[GetCollisionDirection(rayPos - rayPosRounded)];
      i = 5.0f;
      showIndicator = true;
    }
  }

  if(!isTouchingSomething){isGrounded = false;}

  //Teleport the player to the last block they stood on if they fall from the map.
  //This might not save the player from digging straight down.
  //It's mainly here because if the game freezes for too long the player might fall through the map.
  if(glm::distance(playerPos, lastKnownPosition) > 60.0f * sqrt(2)){
    velocity = glm::vec3(0.0f, 0.0f, 0.0f);
    lastKnownPosition.y = lastKnownPosition.y + 3.0f;
    playerPos = lastKnownPosition;
  }

  if(!wasGrounded && isGrounded){Landed();}
  wasGrounded = isGrounded;
  
  return glm::vec3(playerPos.x, playerPos.y + 0.72f, playerPos.z);
}

glm::vec3 Physics::Lerp(glm::vec3 targetVelocity, glm::vec3 currentVelocity, float weight){
  glm::vec3 interpolatedVector = currentVelocity;
  glm::vec3 difference = targetVelocity - currentVelocity;

  if(targetVelocity.x != currentVelocity.x){
    interpolatedVector.x += weight * difference.x;
  }
  if(targetVelocity.y != currentVelocity.y){
    interpolatedVector.y += weight * difference.y;
  }
  if(targetVelocity.z != currentVelocity.z){
    interpolatedVector.z += weight * difference.z;
  }
  return interpolatedVector;
}
