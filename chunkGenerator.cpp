#include "chunkGenerator.h"
#include "block.h"
#include <memory>

ChunkGenerator::ChunkGenerator(){
}

ChunkGenerator::~ChunkGenerator(){}

std::shared_ptr<Chunk> ChunkGenerator::GenerateChunk(glm::vec2 chunkCenter, int seed){
  std::shared_ptr<Chunk> chunk =  std::make_shared<Chunk>();
  //Fill the chunk with 0s, otherwise random data is visualised as blocks. The results are different on Linux and Windows.
  chunk->chunkCenter = chunkCenter;
  for(int x = 0; x < 15; x++){
    for(int y = 0; y < 75; y++){
      for(int z = 0; z < 15; z++){
        chunk->Blocks[x][y][z] = NOTHING_BLOCK;
      }
    }
  }

  int X = (int)chunkCenter.x;
  int Z = (int)chunkCenter.y;

  int* pixel = PerlinChunk(X, Z, seed);

  //The flowers and trees aren't generated based on a seed, they're placed after blocks and determined by modulars.
  int yellowFlower = 0;
  int redFlower = 0;
  int tree = 0;

  for(int x = 0; x < 15; x++){
    for(int z = 0; z < 15; z++){
      int y = pixel[x + z * 15];
      if(y < 10){
        //Sand is placed if the height map shows a number lower than 10.
        chunk->Blocks[x][y][z] = SAND_BLOCK;
        //If sand is placed its above is filled with water until it reaches a height of 10.
        if(y < 9){
          for(int upY = y + 1; upY < 10; upY++){
            chunk->Blocks[x][upY][z] = WATER_BLOCK;
          }
        }
      }
      else{
        //Grass is placed if the height map shows a number higher than 9.
        chunk->Blocks[x][y][z] = GRASS_BLOCK;
        //Trees and flowers are placed depending on very predictable incrementations.
        //Trees are only placed if they won't affect side chunks. This might get changed to increase tree count.
        if(yellowFlower % 5 == 0 && y % 3 == 0){
          chunk->Blocks[x][y + 1][z] = YELLOW_FLOWER_BLOCK;
          yellowFlower++;
        }
        if(redFlower % 6 == 0 && y % 4 == 0){
          chunk->Blocks[x][y + 1][z] = RED_FLOWER_BLOCK;
          redFlower++;
        }
        if(tree % 7 == 0 && y % 5 == 0){
          if(x != 0 && x != 14 && z != 0 && z != 14){
            chunk->Blocks[x][y][z] = DIRT_BLOCK;
            chunk->Blocks[x][y + 1][z] = LOG_BLOCK;
            chunk->Blocks[x][y + 2][z] = LOG_BLOCK;
            chunk->Blocks[x][y + 3][z] = LOG_BLOCK;
            chunk->Blocks[x][y + 4][z] = LOG_BLOCK;
            chunk->Blocks[x][y + 5][z] = LEAVES_BLOCK;
            for(int xX = x - 1; xX <= x + 1; xX++){
              for(int yY = y + 2; yY < y + 6; yY++){
                for(int zZ = z - 1; zZ <= z + 1; zZ++){
                  if(xX != x || zZ != z){
                    if(chunk->Blocks[xX][yY][zZ] == NOTHING_BLOCK){chunk->Blocks[xX][yY][zZ] = LEAVES_BLOCK;}
                  }
                }
              }
            }
          }
          tree++;
        }
      }
      //Dirt is placed one block below the grass/sand and the rest is filled with stones.
      for(int downY = y - 1; downY >= 0; downY--){
        if(downY - y == -1){chunk->Blocks[x][downY][z] = DIRT_BLOCK;}
        else{chunk->Blocks[x][downY][z] = STONE_BLOCK;}
      }
    }
  }

  delete pixel;
  GenerateFaces(chunkCenter, chunk, seed);
  chunk->isInit = true;
  return chunk;
}

void ChunkGenerator::GenerateFaces(glm::vec2 chunkCenter, std::shared_ptr<Chunk> chunk, int seed){
  int* rightChunk = PerlinChunk((int)(chunkCenter.x + 15.0f), (int)chunkCenter.y, seed);
  int* leftChunk = PerlinChunk((int)(chunkCenter.x - 15.0f), (int)chunkCenter.y, seed);
  int* frontChunk = PerlinChunk((int)(chunkCenter.x), (int)(chunkCenter.y + 15.0f), seed);
  int* behindChunk = PerlinChunk((int)(chunkCenter.x), (int)(chunkCenter.y - 15.0f), seed);
  for(int x = 0; x < 15; x++){
    for(int y = 0; y < 75; y++){
      for(int z = 0; z < 15; z++){
        if(chunk->Blocks[x][y][z] != 0){
          Block tempBlock = Block(chunk->Blocks[x][y][z], glm::vec3((int)chunkCenter.x - 7 + x, y, (int)chunkCenter.y - 7 + z));

          if(chunk->Blocks[x][y][z] == WATER_BLOCK){
            if(chunk->Blocks[x][y + 1][z] == 0){
              chunk->waterFaces.push_back(tempBlock.faceTop);
            }
          }else if(chunk->Blocks[x][y][z] == YELLOW_FLOWER_BLOCK || chunk->Blocks[x][y][z] == RED_FLOWER_BLOCK){
            chunk->foliageFaces.push_back(tempBlock.faceFront);
            chunk->foliageFaces.push_back(tempBlock.faceBehind);
          }else if(chunk->Blocks[x][y][z] == LEAVES_BLOCK){
            if(y != 0){
              if(CheckBlockType(LEAVES, chunk->Blocks[x][y - 1][z])){chunk->foliageFaces.push_back(tempBlock.faceBottom);}
            }

            if(y != 74){
              if(CheckBlockType(LEAVES, chunk->Blocks[x][y + 1][z])){chunk->foliageFaces.push_back(tempBlock.faceTop);}
            }else{chunk->foliageFaces.push_back(tempBlock.faceTop);}

            if(x != 0){
              if(CheckBlockType(LEAVES, chunk->Blocks[x - 1][y][z])){chunk->foliageFaces.push_back(tempBlock.faceLeft);}
            }else{
              if(leftChunk[14 + z * 15] < y){chunk->foliageFaces.push_back(tempBlock.faceLeft);}
            }

            if(x != 14){
              if(CheckBlockType(LEAVES, chunk->Blocks[x + 1][y][z])){chunk->foliageFaces.push_back(tempBlock.faceRight);}
            }else{
              if(rightChunk[0 + z * 15] < y){chunk->foliageFaces.push_back(tempBlock.faceRight);}
            }

            if(z != 0){
              if(CheckBlockType(LEAVES, chunk->Blocks[x][y][z - 1])){chunk->foliageFaces.push_back(tempBlock.faceBehind);}
            }else{
              if(behindChunk[x + 14 * 15] < y){chunk->foliageFaces.push_back(tempBlock.faceBehind);}
            }

            if(z != 14){
              if(CheckBlockType(LEAVES, chunk->Blocks[x][y][z + 1])){chunk->foliageFaces.push_back(tempBlock.faceFront);}
            }else{
              if(frontChunk[x + 0 * 15] < y){chunk->foliageFaces.push_back(tempBlock.faceFront);}
            }
          }else{

            if(y != 0){
              if(CheckBlockType(BLOCK, chunk->Blocks[x][y - 1][z])){chunk->faces.push_back(tempBlock.faceBottom);}
            }

            if(y != 74){
              if(CheckBlockType(BLOCK, chunk->Blocks[x][y + 1][z])){chunk->faces.push_back(tempBlock.faceTop);}
            }else{
              chunk->faces.push_back(tempBlock.faceTop);
            }

            if(x != 0){
              if(CheckBlockType(BLOCK, chunk->Blocks[x - 1][y][z])){chunk->faces.push_back(tempBlock.faceLeft);}
            }else{
              if(leftChunk[14 + z * 15] < y){chunk->faces.push_back(tempBlock.faceLeft);}
            }

            if(x != 14){
              if(CheckBlockType(BLOCK, chunk->Blocks[x + 1][y][z])){chunk->faces.push_back(tempBlock.faceRight);}
            }else{
              if(rightChunk[0 + z * 15] < y){chunk->faces.push_back(tempBlock.faceRight);
              }
            }


            if(z != 0){
              if(CheckBlockType(BLOCK, chunk->Blocks[x][y][z - 1])){chunk->faces.push_back(tempBlock.faceBehind);}
            }else{
              if(behindChunk[x + 14 * 15] < y){chunk->faces.push_back(tempBlock.faceBehind);}
            }

            if(z != 14){
              if(CheckBlockType(BLOCK, chunk->Blocks[x][y][z + 1])){chunk->faces.push_back(tempBlock.faceFront);}
            }else{
              if(frontChunk[x + 0 * 15] < y){chunk->faces.push_back(tempBlock.faceFront);}
            }
          }
        }
      }
    }
  }

  delete leftChunk;
  delete rightChunk;
  delete frontChunk;
  delete behindChunk;
}

bool ChunkGenerator::CheckBlockType(BlockType thisBlockType, int otherBlockID){
  if(thisBlockType == BLOCK){
    if(otherBlockID == NOTHING_BLOCK || otherBlockID == WATER_BLOCK || otherBlockID == YELLOW_FLOWER_BLOCK || otherBlockID == RED_FLOWER_BLOCK || otherBlockID == LEAVES_BLOCK){
      return true;
    }else{return false;}
  }
  if(thisBlockType == LEAVES){
    if(otherBlockID == NOTHING_BLOCK || otherBlockID == WATER_BLOCK || otherBlockID == YELLOW_FLOWER_BLOCK || otherBlockID == RED_FLOWER_BLOCK){
      return true;
    }else{return false;}
  }
  return false;
}


//The rest is 2D perlin noise generation for height map. This isn't my code, it's credited in the README.
int* ChunkGenerator::PerlinChunk(int X, int Z, int seed){
  const int chunkW = 15;
  const int chunkH = 15;

  int* pixels = new int[chunkW * chunkH];

  const int gridSize = 50;

  for(int x = 0; x < chunkW; x++){
    for(int y = 0; y < chunkH; y++){
      int index = y * chunkW + x;
      float val = 0;
      float freq = 0.75;
      float amp = 1;
      for(int i = 0; i < 12; i++){
        //The +1000000s here are because chunk center coordinates are used for noise generation and this doesn't work with negative numbers.
        val += perlin((x + X + 1000000) * freq / gridSize, (y + Z + 1000000) * freq / gridSize, seed) * amp;
        freq *= 2;
        amp /= 2;
      }
      val *= 1.2f;
      if(val > 1.0f){val = 1.0f;}
      else if(val < -1.0f){val = -1.0f;}
      int color = (int)(((val + 1.0f) * 0.5f) * 40);
      pixels[index] = color;
    }
  }
  return pixels;
}

glm::vec2 ChunkGenerator::randomGradient(int ix, int iy, int seed){
  const unsigned w = 8 * sizeof(unsigned);
  const unsigned s = w/2;
  unsigned a = ix, b = iy;
  a *= seed;
  b ^= a << s | a >> w - s;
  b *= 4372899238;
  a ^= b << s | b >> w - s;
  a *= 2048419325;
  float random = a * (3.14159265 / ~(~0u >> 1));

  glm::vec2 v;
  v.x = sin(random);
  v.y = cos(random);

  return v;
}

float ChunkGenerator::dotGridGradient(int ix, int iy, float x, float y, int seed){
  glm::vec2 gradient = randomGradient(ix, iy, seed);
  float dx = x - (float)ix;
  float dy = y - (float)iy;
  return(dx * gradient.x + dy * gradient.y);
}

float ChunkGenerator::interpolate(float a0, float a1, float w){
  return (a1 - a0) * (3.0f - w * 2.0f) * w * w + a0;
}

float ChunkGenerator::perlin(float x, float y, int seed){
  int x0 = (int)x;
  int y0 = (int)y;

  int x1 = x0 + 1;
  int y1 = y0 + 1;
  float sx = x - (float)x0;
  float sy = y - (float)y0;

  float n0 = dotGridGradient(x0, y0, x, y, seed);
  float n1 = dotGridGradient(x1, y0, x, y, seed);
  float ix0 = interpolate(n0, n1, sx);
  n0 = dotGridGradient(x0, y1, x, y, seed);
  n1 = dotGridGradient(x1, y1, x, y, seed);
  float ix1 = interpolate(n0, n1, sx);

  float value = interpolate(ix0, ix1, sy);

  return value;
}
