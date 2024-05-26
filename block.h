#ifndef BLOCK_H
#define BLOCK_H

#include "glm/glm.hpp"

enum BlockNames{
  NOTHING_BLOCK,
  DIRT_BLOCK,
  GRASS_BLOCK,
  STONE_BLOCK,
  DEBUG_BLOCK,
  SAND_BLOCK,
  WATER_BLOCK,
  YELLOW_FLOWER_BLOCK,
  RED_FLOWER_BLOCK,
  LOG_BLOCK,
  LEAVES_BLOCK
};

struct Vertex{
  glm::vec3 pos;
  glm::vec2 tex;
  glm::vec3 norm;

  Vertex(){}

  Vertex(const glm::vec3 &pos_, const glm::vec2 &tex_, const glm::vec3 &norm_){
    pos = pos_;
    tex = tex_;
    norm = norm_;
  }
};

struct Triangle{
  Vertex one;
  Vertex two;
  Vertex three;
  Triangle(){}
  Triangle(const Vertex &one_, const Vertex &two_, const Vertex &three_){
    one = one_;
    two = two_;
    three = three_;
  }
};

struct Face{
  Triangle botLeft;
  Triangle topRight;
  Face(){}
  Face(const Vertex &topLeft_, const Vertex &botLeft_, const Vertex &botRight_, const Vertex &topRight_){
    botLeft = Triangle(topLeft_, botLeft_, botRight_);
    topRight = Triangle(botRight_, topRight_, topLeft_);
  }
};

struct Block{
  Face faceFront;
  Face faceBehind;
  Face faceTop;
  Face faceBottom;
  Face faceLeft;
  Face faceRight;
  glm::vec3 blockPos;
  unsigned int blockID;
  Block(){}
  Block(unsigned int blockID_, glm::vec3 blockPos_){
    blockID = blockID_;
    blockPos = blockPos_;
    glm::vec3 leftTopFront = glm::vec3(blockPos.x - 0.5f, blockPos.y + 0.5f, blockPos.z + 0.5f);
    glm::vec3 leftBottomFront = glm::vec3(blockPos.x - 0.5f, blockPos.y - 0.5f, blockPos.z + 0.5f);
    glm::vec3 rightBottomFront = glm::vec3(blockPos.x + 0.5f, blockPos.y - 0.5f, blockPos.z + 0.5f);
    glm::vec3 rightTopFront = glm::vec3(blockPos.x + 0.5f, blockPos.y + 0.5f, blockPos.z + 0.5f);

    glm::vec3 leftTopBehind = glm::vec3(blockPos.x - 0.5f, blockPos.y + 0.5f, blockPos.z - 0.5f);
    glm::vec3 leftBottomBehind = glm::vec3(blockPos.x - 0.5f, blockPos.y - 0.5f, blockPos.z - 0.5f);
    glm::vec3 rightBottomBehind = glm::vec3(blockPos.x + 0.5f, blockPos.y - 0.5f, blockPos.z - 0.5f);
    glm::vec3 rightTopBehind = glm::vec3(blockPos.x + 0.5f, blockPos.y + 0.5f, blockPos.z - 0.5f);
    if(blockID == WATER_BLOCK){
      leftTopFront.y = leftTopFront.y - 0.0625f;
      rightTopFront.y = rightTopFront.y - 0.0625f;
      leftTopBehind.y = leftTopBehind.y - 0.0625f;
      rightTopBehind.y = rightTopBehind.y - 0.0625f;
    }

    glm::vec2 leftTop;
    glm::vec2 leftBottom;
    glm::vec2 rightBottom;
    glm::vec2 rightTop;

    glm::vec3 normalFront = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 normalBehind = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 normalTop = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 normalBottom = glm::vec3(0.0f, -1.0f, 0.0f);
    glm::vec3 normalLeft = glm::vec3(-1.0f, 0.0f, 0.0f);
    glm::vec3 normalRight = glm::vec3(1.0f, 0.0f, 0.0f);
    
    float texCenterX;
    float texCenterY;
    
    if(blockID == DIRT_BLOCK){
      texCenterX = 0;
      texCenterY = 0;
    }else if(blockID == STONE_BLOCK){
      texCenterX = 3;
      texCenterY = 0;
    }else if(blockID == DEBUG_BLOCK){
      texCenterX = 4;
      texCenterY = 0;
    }else if(blockID == SAND_BLOCK){
      texCenterX = 5;
      texCenterY = 0;
    }else if(blockID == WATER_BLOCK){
      texCenterX = 0;
      texCenterY = 1;
    }else if(blockID == YELLOW_FLOWER_BLOCK){
      texCenterX = 2;
      texCenterY = 1;
    }else if(blockID == RED_FLOWER_BLOCK){
      texCenterX = 3;
      texCenterY = 1;
    }else if(blockID == LEAVES_BLOCK){
      texCenterX = 0;
      texCenterY = 2;
    }

    texCenterX = 0.08f + texCenterX * 0.16f;
    texCenterY = 1.0f - 0.08f - texCenterY * 0.16f;

    leftTop = glm::vec2(texCenterX - 0.08f, texCenterY + 0.08f);
    leftBottom = glm::vec2(texCenterX - 0.08f, texCenterY - 0.08f);
    rightBottom = glm::vec2(texCenterX + 0.08f, texCenterY - 0.08f);
    rightTop = glm::vec2(texCenterX + 0.08f, texCenterY + 0.08f);

    glm::vec2 sideLeftTop = leftTop;
    glm::vec2 sideLeftBottom = leftBottom;
    glm::vec2 sideRightBottom = rightBottom;
    glm::vec2 sideRightTop = rightTop;
    glm::vec2 bottomLeftTop = leftTop;
    glm::vec2 bottomLeftBottom = leftBottom;
    glm::vec2 bottomRightBottom = rightBottom;
    glm::vec2 bottomRightTop = rightTop;
    glm::vec2 topLeftTop = leftTop;
    glm::vec2 topLeftBottom = leftBottom;
    glm::vec2 topRightBottom = rightBottom;
    glm::vec2 topRightTop = rightTop;

    if(blockID == GRASS_BLOCK){
      sideLeftTop = glm::vec2(0.32f, 1.0f);
      sideLeftBottom = glm::vec2(0.32f, 1.0f - 0.16f);
      sideRightBottom = glm::vec2(0.48f, 1.0f - 0.16f);
      sideRightTop = glm::vec2(0.48f, 1.0f);
      bottomLeftTop = glm::vec2(0.0f, 1.0f);
      bottomLeftBottom = glm::vec2(0.0f, 1.0f -0.16f);
      bottomRightBottom = glm::vec2(0.16f, 1.0f - 0.16f);
      bottomRightTop = glm::vec2(0.16f, 1.0f);
      topLeftTop = glm::vec2(0.16f, 1.0f);
      topLeftBottom = glm::vec2(0.16f, 1.0f - 0.16f);
      topRightBottom = glm::vec2(0.32f, 1.0f - 0.16f);
      topRightTop = glm::vec2(0.32f, 1.0f);
    }else if(blockID == LOG_BLOCK){
      sideLeftTop = glm::vec2(0.64, 1.0f - 0.16f);
      sideLeftBottom = glm::vec2(0.64f, 1.0f - 0.32f);
      sideRightBottom = glm::vec2(0.8f, 1.0f - 0.32f);
      sideRightTop = glm::vec2(0.8f, 1.0f - 0.16f);
      bottomLeftTop = glm::vec2(0.8f, 1.0f - 0.16f);
      bottomLeftBottom = glm::vec2(0.8f, 1.0f - 0.32f);
      bottomRightBottom = glm::vec2(0.96f, 1.0f - 0.32f);
      bottomRightTop = glm::vec2(0.96, 1.0f - 0.16f);
      topLeftTop = bottomLeftTop;
      topLeftBottom = bottomLeftBottom;
      topRightBottom = bottomRightBottom;
      topRightTop = bottomRightTop;
    }

    if(blockID == YELLOW_FLOWER_BLOCK || blockID == RED_FLOWER_BLOCK){
      faceFront = Face(Vertex(leftTopFront, sideLeftTop, normalFront), Vertex(leftBottomFront, sideLeftBottom, normalFront), Vertex(rightBottomBehind, sideRightBottom, normalFront), Vertex(rightTopBehind, sideRightTop, normalFront));
      faceBehind = Face(Vertex(rightTopFront, sideLeftTop, normalBehind), Vertex(rightBottomFront, sideLeftBottom, normalBehind), Vertex(leftBottomBehind, sideRightBottom, normalBehind), Vertex(leftTopBehind, sideRightTop, normalBehind));
    }else{
      faceFront = Face(Vertex(leftTopFront, sideLeftTop, normalFront), Vertex(leftBottomFront, sideLeftBottom, normalFront), Vertex(rightBottomFront, sideRightBottom, normalFront), Vertex(rightTopFront, sideRightTop, normalFront));
      faceBehind = Face(Vertex(rightTopBehind, sideLeftTop, normalBehind), Vertex(rightBottomBehind, sideLeftBottom, normalBehind), Vertex(leftBottomBehind, sideRightBottom, normalBehind), Vertex(leftTopBehind, sideRightTop, normalBehind));
    }

    if(blockID != YELLOW_FLOWER_BLOCK && blockID != RED_FLOWER_BLOCK){
      faceRight = Face(Vertex(rightTopFront, sideLeftTop, normalRight), Vertex(rightBottomFront, sideLeftBottom, normalRight), Vertex(rightBottomBehind, sideRightBottom, normalRight), Vertex(rightTopBehind, sideRightTop, normalRight));
      faceLeft = Face(Vertex(leftTopBehind, sideLeftTop, normalLeft), Vertex(leftBottomBehind, sideLeftBottom, normalLeft), Vertex(leftBottomFront, sideRightBottom, normalLeft), Vertex(leftTopFront, sideRightTop, normalLeft));
      faceTop = Face(Vertex(leftTopBehind, topLeftTop, normalTop), Vertex(leftTopFront, topLeftBottom, normalTop), Vertex(rightTopFront, topRightBottom, normalTop), Vertex(rightTopBehind, topRightTop, normalTop));
      faceBottom = Face(Vertex(leftBottomFront, bottomLeftTop, normalBottom), Vertex(leftBottomBehind, bottomLeftBottom, normalBottom), Vertex(rightBottomBehind, bottomRightBottom, normalBottom), Vertex(rightBottomFront, bottomRightTop, normalBottom));
    }

  }
};

#endif // !BLOCK_H
