#include <iostream>
#include "game.h"
#include "resourceManager.h"

void FrameBufferSizeCallback(GLFWwindow* window, int width, int height);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xposIn, double yposIn);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset);

const unsigned int screenWidth = 960;
const unsigned int screenHeight = 540;

float lastX = screenWidth / 2.0f;
float lastY = screenHeight / 2.0f;
bool firstMouse = true;
float deltaAdd = 0.0f;

Game cubeGame(screenWidth, screenHeight);

int main(){
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, true);

  GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "CubesGame", NULL, NULL);
  glfwMakeContextCurrent(window);


  if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
    std::cout << "GLAD failed to initialize" << std::endl;
    return -1;
  }
  glfwSetKeyCallback(window, KeyCallback);
  glfwSetFramebufferSizeCallback(window, FrameBufferSizeCallback);
  glfwSetMouseButtonCallback(window, MouseButtonCallback);
  glfwSetCursorPosCallback(window, MouseCallback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetScrollCallback(window, ScrollCallback);

  glViewport(0, 0, screenWidth, screenHeight);
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  float deltaTime = 0.0f;
  float lastFrame = 0.0f;

  cubeGame.Init();

  while(!glfwWindowShouldClose(window)){
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    glfwPollEvents();
    
    cubeGame.ProcessInput(deltaTime);

    deltaAdd += deltaTime;
    //PhysicsUpdate runs at 30 fps.
    while(deltaAdd >= 0.033333333f){
      cubeGame.PhysicsUpdate(deltaTime);
      deltaAdd -= 0.033333333f;
    }
    //Update runs every frame.
    cubeGame.Update(deltaTime);

    glClearColor(0.0f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    cubeGame.Render();
    lastFrame = currentFrame;
    glfwSwapBuffers(window);
  }

  ResourceManager::Clear();
  cubeGame.Clear();

  glfwTerminate();

  return 0;

}

void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset){
  if(yOffset > 0.0f){
    cubeGame.MouseScroll(true);
  }
  if(yOffset < 0.0f){
    cubeGame.MouseScroll(false);
  }
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode){
  if(key == GLFW_KEY_Q && action == GLFW_PRESS){
    glfwSetWindowShouldClose(window, true);
  }

  if(key >= 0 && key < 1024){
    if(action == GLFW_PRESS){
      cubeGame.keys[key] = true;
    }else if(action == GLFW_RELEASE){
      cubeGame.keys[key] = false;
    }
  }
}


void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods){
  if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
    cubeGame.BreakBlock();
  }
  if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS){
    cubeGame.PlaceBlock();
  }
}


void FrameBufferSizeCallback(GLFWwindow* window, int width, int height){
  glViewport(0, 0, width, height);
}

void MouseCallback(GLFWwindow* window, double xposIn, double yposIn){
  float xPos = static_cast<float>(xposIn);
  float yPos = static_cast<float>(yposIn);

  if(firstMouse){
    lastX = xPos;
    lastY = yPos;
    firstMouse = false;
  }
  float xOffset = xPos - lastX;
  float yOffset = lastY - yPos;
  lastX = xPos;
  lastY = yPos;
  
  cubeGame.UpdateCamera(xOffset, yOffset);
}
