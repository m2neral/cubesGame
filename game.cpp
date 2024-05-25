#include "game.h"
#include "blockIndicator.h"
#include <future>

Camera camera(glm::vec3(0.0f, 50.0f, 0.0f));
ChunkManager* chunkManager;
BlockIndicator* blockIndicator;
int blockPlaceID = DIRT_BLOCK;
Physics physics;

glm::vec2 lastPos;

glm::mat4 projection;
glm::mat4 view;

bool isGenBusy = false;
bool isAskForCollisionCoords = false;

std::vector<std::future<void>> futuresVec;

Game::Game(unsigned int width_, unsigned int height_): width(width_), height(height_){}

Game::~Game(){}

void Game::Init(){
  ResourceManager::LoadShader("blockIndicator.vert", "blockIndicator.frag", nullptr, "IndicatorShader");
  this->indicatorShader = ResourceManager::GetShader("IndicatorShader");
  blockIndicator = new BlockIndicator();
  blockIndicator->Init();

  chunkManager = new ChunkManager();
  chunkManager->Init();
  physics.initial = false;
}

void Game::ChunkGen(){
  //I know the empty while loop looks wrong but this ensures each thread waits for its turn,
  //executes what it's supposed to as soon as it's available,
  //avoids mutex locking (therefore also avoiding most renderer freezes if the chunk generation takes too long)
  //and keeps memory usage really low (since it avoids copying the chunk pointers all the time (more on that in chunkManager.GenerateAroundCam)).
  //I'm sure there's a better way to achieve this but it works very reliably in this situation.
  isGenBusy = true;
  while(chunkManager->isRendererBusy){}
  chunkManager->GenerateAroundCam(glm::vec2(camera.GetPositionVec3().x, camera.GetPositionVec3().z));
  isGenBusy = false;
}

void Game::AsyncChunkGen(){
  futuresVec.push_back(std::async(std::launch::async, ChunkGen));
}

void Game::ProcessInput(float deltaTime){
  if(this->keys[GLFW_KEY_1]){blockPlaceID = DIRT_BLOCK;}//Dirt
  if(this->keys[GLFW_KEY_2]){blockPlaceID = GRASS_BLOCK;}//Grass
  if(this->keys[GLFW_KEY_3]){blockPlaceID = STONE_BLOCK;}//Stone
  if(this->keys[GLFW_KEY_4]){blockPlaceID = DEBUG_BLOCK;}//Debug block
  if(this->keys[GLFW_KEY_5]){blockPlaceID = SAND_BLOCK;}//Sand
  if(this->keys[GLFW_KEY_6]){blockPlaceID = LOG_BLOCK;}//Log
  

  if(this->keys[GLFW_KEY_W]){physics.walkForward = true;}
  else if(!this->keys[GLFW_KEY_W]){physics.walkForward = false;}

  if(this->keys[GLFW_KEY_S]){physics.walkBackward = true;}
  else if(!this->keys[GLFW_KEY_S]){physics.walkBackward = false;}

  if(this->keys[GLFW_KEY_A]){physics.walkLeft = true;}
  else if(!this->keys[GLFW_KEY_A]){physics.walkLeft = false;}

  if(this->keys[GLFW_KEY_D]){physics.walkRight = true;}
  else if(!this->keys[GLFW_KEY_D]){physics.walkRight = false;}

  if(this->keys[GLFW_KEY_SPACE]){physics.shouldJump = true;}
  else if(!this->keys[GLFW_KEY_SPACE]){physics.shouldJump = false;}

}

//Block breaking and placing is achieved by setting the position and blockID in a separate array in the renderer class. The renderer adds or removes the relevant faces blockIDs while it's going through each chunk to render them.
void Game::BreakBlock(){
  if(physics.showIndicator){
    chunkManager->SetBlockToBeBroken(blockIndicatorPos);
  }
}

void Game::PlaceBlock(){
  if(physics.showIndicator){
    glm::vec3 tempBlockCoords = blockIndicatorPos + blockIndicatorDir;
    if(tempBlockCoords.y >= 0 && tempBlockCoords.y <= 74){
      chunkManager->SetBlockToBePlaced(tempBlockCoords, blockPlaceID);
    }
  }
}

void Game::PhysicsUpdate(float deltaTime){
  //Input is processed to determine movement direction.
  physics.ProcessWalk();
  //The player position is set by physics.
  physicsPosition = physics.Update(camera.GetFrontVec3(), camera.getFakeFrontVec3(), deltaTime, chunkManager->collisionCoords);
  //The block indicator's position and direction are set by the physics loop while it goes through each collision object near the player.
  blockIndicatorPos = physics.indicatorPosition;
  blockIndicatorDir = physics.indicatorDirection;

  chunkManager->RequestCollisionCoords();
  //If the player walked 30.0f (excluding the Y axis), chunk generation is triggered around the player.
  if(glm::distance(glm::vec2(physicsPosition.x, physicsPosition.z), lastPos) > 30.0f && !isGenBusy){
    lastPos = glm::vec2(physicsPosition.x, physicsPosition.z);
    AsyncChunkGen();
  }

}

void Game::Update(float deltaTime){
  //The player position is set at 30fps. The camera position is then interpolated every frame for smooth movement regardless of framerate.
  camera.SetPositionVec3(physics.Lerp(physicsPosition, camera.GetPositionVec3(), 20.0f * deltaTime));
  projection = glm::perspective(glm::radians(camera.Zoom), static_cast<float>(width) / static_cast<float>(height), 0.1f, 500.0f);
  view = camera.GetViewMatrix();
}

void Game::Render(){
  chunkManager->Update(projection, view, camera.GetPositionVec3(), camera.GetFrontVec3(), isGenBusy);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  DrawIndicator();
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Game::DrawIndicator(){
  if(physics.showIndicator){
    this->indicatorShader.Use();
    this->indicatorShader.SetMat4("projection", projection);
    this->indicatorShader.SetMat4("view", view);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, blockIndicatorPos);
    model = glm::scale(model, glm::vec3(1.01f, 1.01f, 1.01f));
    this->indicatorShader.SetMat4("model", model);
    blockIndicator->Render();
  }
}

void Game::UpdateCamera(float xOffset, float yOffset){
  camera.ProcessMouseMovement(xOffset, yOffset);
}

void Game::Clear(){
  delete blockIndicator;
}
