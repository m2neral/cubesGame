#include "chunkManager.h"
#include "block.h"
#include "chunkGenerator.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <utility>

int renderDistance = 12;
float renderDistanceInChunks = 15.0f * (float)renderDistance;
float maxDistance = renderDistanceInChunks + 15.0f;

ChunkManager::ChunkManager(Shader &shader_, Shader &waterShader_, Shader &foliageShader_, Texture2D &texture_){
  shader = shader_;
  waterShader = waterShader_;
  foliageShader = foliageShader_;
  texture = texture_;
  time_t t;
  time(&t);
  seed = difftime(t, 0);
}

ChunkManager::~ChunkManager(){}

void ChunkManager::SetBlockToBeBroken(glm::vec3 blockCoord){
  chunkOfBlockToBeBroken = GetRelatedChunkCenter(glm::vec2(blockCoord.x, blockCoord.z));
  blockToBeBroken = blockCoord;
  isBlockBreak = true;
}

void ChunkManager::SetBlockToBePlaced(glm::vec3 blockCoord, int blockID){
  chunkOfBlockToBePlaced = GetRelatedChunkCenter(glm::vec2(blockCoord.x, blockCoord.z));
  blockToBePlaced = blockCoord;
  blockToBePlacedID = blockID;
  isBlockPlace = true;
}

void ChunkManager::RequestCollisionCoords(){
  if(tempCollisionCoords.empty() == false){
    collisionCoords->clear();
    collisionCoords->swap(tempCollisionCoords);
    tempCollisionCoords = std::vector<glm::vec3>();
  }
  didGameRequestCollisionCoords = true;
}

void ChunkManager::Init(){
  GenerateAroundCam(glm::vec2(0.0f, 0.0f));
  collisionCoords = std::make_shared<std::vector<glm::vec3>>();
}

//Except for the initial generation, this runs asynchronously. Creating a new ChunkGenerator pointer and deleting it afterwards is to make sure there are no unused block faces taking up space in memory.
//The currentChunkCenters vector is filled by the Update function. Setting the appropriate bools at the right time and using the empty while loop in game.ChunkGen makes sure the vector isn't accessed by multiple threads at the same time.
//Keeping the center coordinates in a separate vector for the generator to look at also lets the renderer do other operations on the chunks without being interrupted.
void ChunkManager::GenerateAroundCam(glm::vec2 camPos){
  ChunkGenerator* chunkGenerator = new ChunkGenerator;
  glm::vec2 center = GetRelatedChunkCenter(camPos);
  for(int x = (int)center.x - (int)maxDistance; x <= (int)center.x + (int)maxDistance; x = x + 15){
    for(int z = (int)center.y - (int)maxDistance; z <= (int)center.y + (int)maxDistance; z = z + 15){
      glm::vec2 tempVec = glm::vec2((float)x, (float)z);
      if(glm::distance(tempVec, camPos) < maxDistance){
        if(std::find(currentChunkCenters.begin(), currentChunkCenters.end(), tempVec) == currentChunkCenters.end()){
          newlyGeneratedChunks.push_back(chunkGenerator->GenerateChunk(tempVec, seed));
        }
      }
    }
  }
  delete chunkGenerator;
}

void ChunkManager::Update(glm::mat4 projection, glm::mat4 view, glm::vec3 camPos, glm::vec3 camFront, bool isGenBusy){

  //Setting isRendererBusy here makes sure the generator doesn't attempt to access chunk centers while this is happening.
  //Chunks are initialized here if they were recently created.
  //Then the center coordinates of each chunk that are near or in-front of the player are stored in a vector to get rendered.
  //This check also destroys far-away chunks.
  isRendererBusy = true;
  if(!isGenBusy){
    std::vector<glm::vec2> chunkCentersForDeleting;
    chunkCentersForDeleting.clear();

    currentChunkCenters = std::vector<glm::vec2>();
    chunkCentersForRendering = std::vector<glm::vec2>();
    for(auto it : newlyGeneratedChunks){
      chunks[it->chunkCenter] = it;
    }

    newlyGeneratedChunks = std::vector<std::shared_ptr<Chunk>>();

    for(auto it : chunks){
      if(it.second != NULL){
        if(it.second->isInit){
          it.second->isInit = false;
          it.second->Init();
        }
        if(glm::distance(it.first, glm::vec2(camPos.x, camPos.z)) > maxDistance){
          chunkCentersForDeleting.push_back(it.first);
        }else{
          currentChunkCenters.push_back(it.first);
          if(glm::distance(it.first, glm::vec2(camPos.x, camPos.z)) < 15.0f * sqrt(2) || glm::dot(camFront, camPos - glm::vec3(it.first.x, 0.0f, it.first.y)) < 0){
            chunkCentersForRendering.push_back(it.first);
          }
        }
      }
    }
    for(auto it : chunkCentersForDeleting){
      chunks.erase(it);
    }
    chunkCentersForDeleting = std::vector<glm::vec2>();
  }
  isRendererBusy = false;
  //Chunk centers are sorted by their distance to the player. This currently doesn't have a use. It is going to be useful for lighting calculations for shaders.
  std::sort(chunkCentersForRendering.begin(), chunkCentersForRendering.end(), [=](glm::vec2 a, glm::vec2 b){
    auto distA = glm::distance(a, glm::vec2(camPos.x, camPos.z));
    auto distB = glm::distance(b, glm::vec2(camPos.x, camPos.z));
    return distA > distB;
  });
  glDisable(GL_BLEND);
  glEnable(GL_CULL_FACE);

  glActiveTexture(GL_TEXTURE0);
  texture.Bind();
  shader.Use();
  shader.SetInt("texture1", 0);
  shader.SetVec3("camNormal", camFront);
  shader.SetMat4("projection", projection);
  shader.SetMat4("view", view);
  glm::mat4 model = glm::mat4(1.0f);
  shader.SetMat4("model", model);


  for(auto it : chunkCentersForRendering){
    if(chunks[it] != NULL){
      //Each chunk is checked to see if they're set to have a face added to or removed from them before being rendered.
      CheckAndUpdateIfFaceAdded(it);
      CheckAndUpdateIfFaceRemoved(it);
      //Same thing but for setting blocks. If the affected blocks are on chunk edge, the appropriate side chunk is set to have its faces updated in the next render loop.
      //The two functions above this are for those checks.
      CheckAndUpdateIfBlockPlaced(it);
      CheckAndUpdateIfBlockBroken(it);

      if(chunks[it]->faces.empty() == false){
        chunks[it]->Render();
        if(didGameRequestCollisionCoords){
          if(glm::distance(it, glm::vec2(camPos.x, camPos.z)) < 15.0f * sqrt(2)){
            //If the chunk is close to the player, its rendered block coordinates are stored for the physics loop to access.
            //There are duplicates here but it gets handled by the physics loop.
            for(auto fit : chunks[it]->faces){
              tempCollisionCoords.push_back(((fit.botLeft.one.pos + fit.botLeft.three.pos) / 2.0f) + (fit.botLeft.one.norm * -0.5f));
            }
          }
        }

      }
    }
  }
  if(didGameRequestCollisionCoords){
    didGameRequestCollisionCoords = false;
  }

  glEnable(GL_BLEND);
  glDisable(GL_CULL_FACE);

  waterShader.Use();
  waterShader.SetInt("texture1", 0);
  waterShader.SetVec3("camNormal", camFront);
  waterShader.SetMat4("projection", projection);
  waterShader.SetMat4("view", view);
  waterShader.SetMat4("model", model);
  for(auto it : chunkCentersForRendering){
    if(chunks[it] != NULL){
      if(chunks[it]->waterFaces.empty() == false){
        chunks[it]->RenderWater();
      }
    }
  }

  foliageShader.Use();
  foliageShader.SetInt("texture1", 0);
  foliageShader.SetVec3("camNormal", camFront);
  foliageShader.SetMat4("projection", projection);
  foliageShader.SetMat4("view", view);
  foliageShader.SetMat4("model", model);
  for(auto it : chunkCentersForRendering){
    if(chunks[it] != NULL){
      if(chunks[it]->foliageFaces.empty() == false){
        chunks[it]->RenderFoliage();
      }
    }
  }
}

//For the block/face update functions, [0] is reserved for the X axis while [1] is reserved for the Z axis.
//This is done to make sure both chunks can update in the same update loop if the updated block is on a corner.
//It is set by the chunk that is being modified if the modified block is on a chunk edge.
//It is then read and used by the chunk that got affected.
void ChunkManager::CheckAndUpdateIfFaceAdded(glm::vec2 chunkCenter){
  for(int i = 0; i <= 1; i++){
    if(faceAddedUpdatingChunks[i].chunkCenter == chunkCenter){
      int indexX = faceAddedUpdatingChunks[i].blockIndex.x;
      int indexY = faceAddedUpdatingChunks[i].blockIndex.y;
      int indexZ = faceAddedUpdatingChunks[i].blockIndex.z;
      glm::vec2 tempChunkStart = chunkCenter - glm::vec2(7.0f, 7.0f);
      if(ChunkGenerator::CheckBlockType(BLOCK, chunks[chunkCenter]->Blocks[indexX][indexY][indexZ]) == false){
        Block tempBlock = Block(chunks[chunkCenter]->Blocks[indexX][indexY][indexZ], glm::vec3(tempChunkStart.x + indexX, indexY, tempChunkStart.y + indexZ));
        if(faceAddedUpdatingChunks[i].blockDirection == CHUNKLEFT){chunks[chunkCenter]->faces.push_back(tempBlock.faceLeft);}
        if(faceAddedUpdatingChunks[i].blockDirection == CHUNKRIGHT){chunks[chunkCenter]->faces.push_back(tempBlock.faceRight);}
        if(faceAddedUpdatingChunks[i].blockDirection == CHUNKFORWARD){chunks[chunkCenter]->faces.push_back(tempBlock.faceFront);}
        if(faceAddedUpdatingChunks[i].blockDirection == CHUNKBACKWARD){chunks[chunkCenter]->faces.push_back(tempBlock.faceBehind);}
      }

      faceAddedUpdatingChunks[i].chunkCenter = glm::vec2(-1.0f, -1.0f);
    }
  }
}

void ChunkManager::CheckAndUpdateIfFaceRemoved(glm::vec2 chunkCenter){
  for(int i = 0; i <= 1; i++){
    if(faceRemovedUpdatingChunks[i].chunkCenter == chunkCenter){
      std::vector<Face> tempVec;
      Face tempFace;
      int indexX = faceRemovedUpdatingChunks[i].blockIndex.x;
      int indexY = faceRemovedUpdatingChunks[i].blockIndex.y;
      int indexZ = faceRemovedUpdatingChunks[i].blockIndex.z;
      glm::vec2 tempChunkStart = chunkCenter - glm::vec2(7.0f, 7.0f);
      if(ChunkGenerator::CheckBlockType(BLOCK, chunks[chunkCenter]->Blocks[indexX][indexY][indexZ]) == false){
        Block tempBlock = Block(chunks[chunkCenter]->Blocks[indexX][indexY][indexZ], glm::vec3(tempChunkStart.x + indexX, indexY, tempChunkStart.y + indexZ));
        if(faceRemovedUpdatingChunks[i].blockDirection == CHUNKLEFT){tempFace = tempBlock.faceLeft;}
        if(faceRemovedUpdatingChunks[i].blockDirection == CHUNKRIGHT){tempFace = tempBlock.faceRight;}
        if(faceRemovedUpdatingChunks[i].blockDirection == CHUNKFORWARD){tempFace = tempBlock.faceFront;}
        if(faceRemovedUpdatingChunks[i].blockDirection == CHUNKBACKWARD){tempFace = tempBlock.faceBehind;}
      }

      for(auto &dit : chunks[chunkCenter]->faces){
        if((dit.botLeft.one.pos == tempFace.botLeft.one.pos && dit.topRight.one.pos == tempFace.topRight.one.pos) == false){
          tempVec.push_back(dit);
        }
      }
      chunks[chunkCenter]->faces = std::vector<Face>();
      chunks[chunkCenter]->faces.swap(tempVec);
      tempVec = std::vector<Face>();
      faceRemovedUpdatingChunks[i].chunkCenter = glm::vec2(-1.0f, -1.0f);
    }
  }
}

void ChunkManager::CheckAndUpdateIfBlockPlaced(glm::vec2 chunkCenter){
  if(isBlockPlace && chunkOfBlockToBePlaced == chunkCenter){
    std::vector<Face> blockPlaceTemporaryDeleteVector;
    std::vector<Face> blockPlaceTemporaryKeepVector;
    std::vector<Face> blockPlaceTemporaryAddVector;
    blockPlaceTemporaryDeleteVector.clear();
    blockPlaceTemporaryKeepVector.clear();
    blockPlaceTemporaryAddVector.clear();
    isBlockPlace = false;
    int intX = (int)blockToBePlaced.x;
    int intY = (int)blockToBePlaced.y;
    int intZ = (int)blockToBePlaced.z;
    int blockIndX = blockToBePlaced.x - (chunkCenter.x - 7.0f);
    int blockIndZ = blockToBePlaced.z - (chunkCenter.y - 7.0f);
    if(chunks[chunkCenter]->Blocks[blockIndX][intY][blockIndZ] == NOTHING_BLOCK){
      chunks[chunkCenter]->Blocks[blockIndX][intY][blockIndZ] = blockToBePlacedID;
      Block tempBlock = Block(blockToBePlacedID, blockToBePlaced);

      if(blockIndX != 0){
        if(ChunkGenerator::CheckBlockType(BLOCK, chunks[chunkCenter]->Blocks[blockIndX - 1][intY][blockIndZ]) == false){
          Block tempBlockX = Block(1, glm::vec3(blockToBePlaced.x - 1, blockToBePlaced.y, blockToBePlaced.z));
          blockPlaceTemporaryDeleteVector.push_back(tempBlockX.faceRight);
        }else{
          blockPlaceTemporaryAddVector.push_back(tempBlock.faceLeft);
        }
      }else{
        if(ChunkGenerator::CheckBlockType(BLOCK, chunks[glm::vec2(chunkCenter.x - 15, chunkCenter.y)]->Blocks[14][intY][blockIndZ])){
          blockPlaceTemporaryAddVector.push_back(tempBlock.faceLeft);
        }else{
          faceRemovedUpdatingChunks[0].chunkCenter = glm::vec2(chunkCenter.x - 15, chunkCenter.y);
          faceRemovedUpdatingChunks[0].blockIndex = glm::vec3(14, intY, blockIndZ);
          faceRemovedUpdatingChunks[0].blockDirection = CHUNKRIGHT;
        }
      }

      if(blockIndX != 14){
        if(ChunkGenerator::CheckBlockType(BLOCK, chunks[chunkCenter]->Blocks[blockIndX + 1][intY][blockIndZ]) == false){
          Block tempBlockX = Block(1, glm::vec3(blockToBePlaced.x + 1, blockToBePlaced.y, blockToBePlaced.z));
          blockPlaceTemporaryDeleteVector.push_back(tempBlockX.faceLeft);
        }else{
          blockPlaceTemporaryAddVector.push_back(tempBlock.faceRight);
        }
      }else{
        if(ChunkGenerator::CheckBlockType(BLOCK, chunks[glm::vec2(chunkCenter.x + 15, chunkCenter.y)]->Blocks[0][intY][blockIndZ])){
          blockPlaceTemporaryAddVector.push_back(tempBlock.faceRight);
        }else{
          faceRemovedUpdatingChunks[0].chunkCenter = glm::vec2(chunkCenter.x + 15, chunkCenter.y);
          faceRemovedUpdatingChunks[0].blockIndex = glm::vec3(0, intY, blockIndZ);
          faceRemovedUpdatingChunks[0].blockDirection = CHUNKLEFT;
        }
      }

      if(intY != 0){
        if(ChunkGenerator::CheckBlockType(BLOCK, chunks[chunkCenter]->Blocks[blockIndX][intY - 1][blockIndZ]) == false){
          Block tempBlockY = Block(1, glm::vec3(blockToBePlaced.x, blockToBePlaced.y - 1, blockToBePlaced.z));
          blockPlaceTemporaryDeleteVector.push_back(tempBlockY.faceTop);
        }else{
          blockPlaceTemporaryAddVector.push_back(tempBlock.faceBottom);
        }
      }
      if(intY != 74){
        if(ChunkGenerator::CheckBlockType(BLOCK, chunks[chunkCenter]->Blocks[blockIndX][intY + 1][blockIndZ]) == false){
          Block tempBlockY = Block(1, glm::vec3(blockToBePlaced.x, blockToBePlaced.y + 1, blockToBePlaced.z));
          blockPlaceTemporaryDeleteVector.push_back(tempBlockY.faceBottom);
        }else{
          blockPlaceTemporaryAddVector.push_back(tempBlock.faceTop);
        }
      }
      if(blockIndZ != 0){
        if(ChunkGenerator::CheckBlockType(BLOCK, chunks[chunkCenter]->Blocks[blockIndX][intY][blockIndZ - 1]) == false){
          Block tempBlockZ = Block(1, glm::vec3(blockToBePlaced.x, blockToBePlaced.y, blockToBePlaced.z - 1));
          blockPlaceTemporaryDeleteVector.push_back(tempBlockZ.faceFront);
        }else{
          blockPlaceTemporaryAddVector.push_back(tempBlock.faceBehind);
        }
      }else{
        if(ChunkGenerator::CheckBlockType(BLOCK, chunks[glm::vec2(chunkCenter.x, chunkCenter.y - 15)]->Blocks[blockIndX][intY][14])){
          blockPlaceTemporaryAddVector.push_back(tempBlock.faceBehind);
        }else{
          faceRemovedUpdatingChunks[1].chunkCenter = glm::vec2(chunkCenter.x, chunkCenter.y - 15);
          faceRemovedUpdatingChunks[1].blockIndex = glm::vec3(blockIndX, intY, 14);
          faceRemovedUpdatingChunks[1].blockDirection = CHUNKFORWARD;
        }
      }
      if(blockIndZ != 14){
        if(ChunkGenerator::CheckBlockType(BLOCK, chunks[chunkCenter]->Blocks[blockIndX][intY][blockIndZ + 1]) == false){
          Block tempBlockZ = Block(1, glm::vec3(blockToBePlaced.x, blockToBePlaced.y, blockToBePlaced.z + 1));
          blockPlaceTemporaryDeleteVector.push_back(tempBlockZ.faceBehind);
        }else{
          blockPlaceTemporaryAddVector.push_back(tempBlock.faceFront);
        }
      }else{
        if(ChunkGenerator::CheckBlockType(BLOCK, chunks[glm::vec2(chunkCenter.x, chunkCenter.y + 15)]->Blocks[blockIndX][intY][0])){
          blockPlaceTemporaryAddVector.push_back(tempBlock.faceFront);
        }else{
          faceRemovedUpdatingChunks[1].chunkCenter = glm::vec2(chunkCenter.x, chunkCenter.y + 15);
          faceRemovedUpdatingChunks[1].blockIndex = glm::vec3(blockIndX, intY, 0);
          faceRemovedUpdatingChunks[1].blockDirection = CHUNKBACKWARD;
        }
      }


      if(blockPlaceTemporaryDeleteVector.empty() == false){
        for(auto fit : chunks[chunkCenter]->faces){
          bool isKeep = true;
          for(auto pit : blockPlaceTemporaryDeleteVector){
            if(fit.botLeft.one.pos == pit.botLeft.one.pos && fit.topRight.one.pos == pit.topRight.one.pos){
              isKeep = false;
            }
          }
          if(isKeep){blockPlaceTemporaryKeepVector.push_back(fit);}
        }
      }else{
        for(auto fit : chunks[chunkCenter]->faces){
          blockPlaceTemporaryKeepVector.push_back(fit);
        }
      }
      if(blockPlaceTemporaryAddVector.empty() == false){
        blockPlaceTemporaryKeepVector.insert(blockPlaceTemporaryKeepVector.end(), blockPlaceTemporaryAddVector.begin(), blockPlaceTemporaryAddVector.end());
      }
      if(blockPlaceTemporaryKeepVector.empty() == false){
        chunks[chunkCenter]->faces = std::vector<Face>();
        chunks[chunkCenter]->faces.swap(blockPlaceTemporaryKeepVector);
      }
    }
    blockPlaceTemporaryDeleteVector = std::vector<Face>();
    blockPlaceTemporaryKeepVector = std::vector<Face>();
    blockPlaceTemporaryAddVector = std::vector<Face>();
  }
}

void ChunkManager::CheckAndUpdateIfBlockBroken(glm::vec2 chunkCenter){
  if(isBlockBreak && chunkOfBlockToBeBroken == chunkCenter){
    std::vector<Face> blockBreakTemporaryDeleteVector;
    std::vector<Face> blockBreakTemporaryKeepVector;
    std::vector<Face> blockBreakTemporaryAddVector;
    blockBreakTemporaryKeepVector.clear();
    blockBreakTemporaryDeleteVector.clear();
    blockBreakTemporaryAddVector.clear();
    isBlockBreak = false;
    int intX = (int)blockToBeBroken.x;
    int intY = (int)blockToBeBroken.y;
    int intZ = (int)blockToBeBroken.z;
    Block tempBlock = Block(DIRT_BLOCK, blockToBeBroken);
    blockBreakTemporaryDeleteVector.push_back(tempBlock.faceTop);
    blockBreakTemporaryDeleteVector.push_back(tempBlock.faceBottom);
    blockBreakTemporaryDeleteVector.push_back(tempBlock.faceLeft);
    blockBreakTemporaryDeleteVector.push_back(tempBlock.faceRight);
    blockBreakTemporaryDeleteVector.push_back(tempBlock.faceFront);
    blockBreakTemporaryDeleteVector.push_back(tempBlock.faceBehind);


    int blockIndX = blockToBeBroken.x - (chunkCenter.x - 7.0f);
    int blockIndZ = blockToBeBroken.z - (chunkCenter.y - 7.0f);


    if(blockIndX != 0){
      if(ChunkGenerator::CheckBlockType(BLOCK, chunks[chunkCenter]->Blocks[blockIndX - 1][intY][blockIndZ]) == false){
        Block tempBlockX = Block(chunks[chunkCenter]->Blocks[blockIndX - 1][intY][blockIndZ], glm::vec3(blockToBeBroken.x - 1, blockToBeBroken.y, blockToBeBroken.z));
        blockBreakTemporaryAddVector.push_back(tempBlockX.faceRight);
      }
    }else{
      faceAddedUpdatingChunks[0].chunkCenter = glm::vec2(chunkCenter.x - 15, chunkCenter.y);
      faceAddedUpdatingChunks[0].blockIndex = glm::vec3(14, intY, blockIndZ);
      faceAddedUpdatingChunks[0].blockDirection = CHUNKRIGHT;
    }

    if(blockIndX != 14){
      if(ChunkGenerator::CheckBlockType(BLOCK, chunks[chunkCenter]->Blocks[blockIndX + 1][intY][blockIndZ]) == false){
        Block tempBlockX = Block(chunks[chunkCenter]->Blocks[blockIndX + 1][intY][blockIndZ], glm::vec3(blockToBeBroken.x + 1, blockToBeBroken.y, blockToBeBroken.z));
        blockBreakTemporaryAddVector.push_back(tempBlockX.faceLeft);
      }
    }else{
      faceAddedUpdatingChunks[0].chunkCenter = glm::vec2(chunkCenter.x + 15, chunkCenter.y);
      faceAddedUpdatingChunks[0].blockIndex = glm::vec3(0, intY, blockIndZ);
      faceAddedUpdatingChunks[0].blockDirection = CHUNKLEFT;
    }

    if(intY != 0){
      if(ChunkGenerator::CheckBlockType(BLOCK, chunks[chunkCenter]->Blocks[blockIndX][intY - 1][blockIndZ]) == false){
        Block tempBlockY = Block(chunks[chunkCenter]->Blocks[blockIndX][intY - 1][blockIndZ], glm::vec3(blockToBeBroken.x, blockToBeBroken.y - 1, blockToBeBroken.z));
        blockBreakTemporaryAddVector.push_back(tempBlockY.faceTop);
      }
    }
    if(intY != 74){
      if(ChunkGenerator::CheckBlockType(BLOCK, chunks[chunkCenter]->Blocks[blockIndX][intY + 1][blockIndZ]) == false){
        Block tempBlockY = Block(chunks[chunkCenter]->Blocks[blockIndX][intY + 1][blockIndZ], glm::vec3(blockToBeBroken.x, blockToBeBroken.y + 1, blockToBeBroken.z));
        blockBreakTemporaryAddVector.push_back(tempBlockY.faceBottom);
      }
    }

    if(blockIndZ != 0){
      if(ChunkGenerator::CheckBlockType(BLOCK, chunks[chunkCenter]->Blocks[blockIndX][intY][blockIndZ - 1]) == false){
        Block tempBlockZ = Block(chunks[chunkCenter]->Blocks[blockIndX][intY][blockIndZ - 1], glm::vec3(blockToBeBroken.x, blockToBeBroken.y, blockToBeBroken.z - 1));
        blockBreakTemporaryAddVector.push_back(tempBlockZ.faceFront);
      }
    }else{
      faceAddedUpdatingChunks[1].chunkCenter = glm::vec2(chunkCenter.x, chunkCenter.y - 15);
      faceAddedUpdatingChunks[1].blockIndex = glm::vec3(blockIndX, intY, 14);
      faceAddedUpdatingChunks[1].blockDirection = CHUNKFORWARD;
    }

    if(blockIndZ != 14){
      if(ChunkGenerator::CheckBlockType(BLOCK, chunks[chunkCenter]->Blocks[blockIndX][intY][blockIndZ + 1]) == false){
        Block tempBlockZ = Block(chunks[chunkCenter]->Blocks[blockIndX][intY][blockIndZ + 1], glm::vec3(blockToBeBroken.x, blockToBeBroken.y, blockToBeBroken.z + 1));
        blockBreakTemporaryAddVector.push_back(tempBlockZ.faceBehind);
      }
    }else{
      faceAddedUpdatingChunks[1].chunkCenter = glm::vec2(chunkCenter.x, chunkCenter.y + 15);
      faceAddedUpdatingChunks[1].blockIndex = glm::vec3(blockIndX, intY, 0);
      faceAddedUpdatingChunks[1].blockDirection = CHUNKBACKWARD;
    }

    chunks[chunkCenter]->Blocks[blockIndX][intY][blockIndZ] = NOTHING_BLOCK;

    if(blockBreakTemporaryDeleteVector.empty() == false){
      for(auto &fit : chunks[chunkCenter]->faces){
        bool isKeep = true;
        for(auto &dit : blockBreakTemporaryDeleteVector){
          if(fit.botLeft.one.pos == dit.botLeft.one.pos && fit.topRight.one.pos == dit.topRight.one.pos){
            isKeep = false;
          }
        }
        if(isKeep){blockBreakTemporaryKeepVector.push_back(fit);}
      }
    }
    if(blockBreakTemporaryAddVector.empty() == false){
      blockBreakTemporaryKeepVector.insert(blockBreakTemporaryKeepVector.end(), blockBreakTemporaryAddVector.begin(), blockBreakTemporaryAddVector.end());
    }
    if(blockBreakTemporaryKeepVector.empty() == false && blockBreakTemporaryDeleteVector.empty() == false){
      chunks[chunkCenter]->faces = std::vector<Face>();
      chunks[chunkCenter]->faces.swap(blockBreakTemporaryKeepVector);
    }
    blockBreakTemporaryKeepVector = std::vector<Face>();
    blockBreakTemporaryDeleteVector = std::vector<Face>();
    blockBreakTemporaryAddVector = std::vector<Face>();
  }
}

glm::vec2 ChunkManager::GetRelatedChunkCenter(glm::vec2 position){
  int roundedX = abs(int(position.x)) + 15 / 2;
  roundedX -= roundedX % 15;
  roundedX = position.x >= 0 ? roundedX : -roundedX;
  int roundedZ = abs(int(position.y)) + 15 / 2;
  roundedZ -= roundedZ % 15;
  roundedZ = position.y >= 0 ? roundedZ : -roundedZ;
  return glm::vec2(roundedX, roundedZ);
}
