#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <map>

#include <tool/shader.h>
#include "camera.h"
#include <geometry/BoxGeometry.h>
#include <geometry/PlaneGeometry.h>
#include <geometry/SphereGeometry.h>

#define STB_IMAGE_IMPLEMENTATION
#include <tool/stb_image.h>

#include <tool/gui.h>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void processInput(GLFWwindow *window);
unsigned int loadTexture(char const *path);
unsigned int loadCubemap(vector<std::string> faces);

// 跳跃逻辑
bool isJumping = false;
float jumpStartTime = 0.0f;
float jumpDuration = 1.0f; // 跳跃总时长
float maxJumpHeight = 2.0f; // 最大跳跃高度


void drawSkyBox(Shader shader, BoxGeometry geometry, unsigned int cubeMap);

std::string Shader::dirName;

int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 600;
// int SCREEN_WIDTH = 1600;
// int SCREEN_HEIGHT = 1200;

// camera value
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

// delta time
float deltaTime = 0.0f;
float lastTime = 0.0f;

float lastX = SCREEN_WIDTH / 2.0f; // 鼠标上一帧的位置
float lastY = SCREEN_HEIGHT / 2.0f;

Camera camera(glm::vec3(0.0, 1.0, 6.0), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);

using namespace std;

int main(int argc, char *argv[])
{
  Shader::dirName = argv[1];
  glfwInit();
  // 设置主要和次要版本
  const char *glsl_version = "#version 330";

  // 片段着色器将作用域每一个采样点（采用4倍抗锯齿，则每个像素有4个片段（四个采样点））
  // glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // 窗口对象
  GLFWwindow *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "LearnOpenGL", NULL, NULL);
  if (window == NULL)
  {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  // -----------------------
  // 创建imgui上下文
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  // 设置样式
  ImGui::StyleColorsDark();
  // 设置平台和渲染器
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  // -----------------------

  // 设置视口
  glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
  glEnable(GL_PROGRAM_POINT_SIZE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // 深度测试
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  // 鼠标键盘事件
  // 1.注册窗口变化监听
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  // 2.鼠标事件
  glfwSetCursorPosCallback(window, mouse_callback);
  // 3.将鼠标隐藏
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  Shader sceneShader("./shader/scene_vert.glsl", "./shader/scene_frag.glsl");
  Shader lightObjectShader("./shader/light_object_vert.glsl", "./shader/light_object_frag.glsl");
  Shader skyboxShader("./shader/cube_map_vert.glsl", "./shader/cube_map_frag.glsl");

  PlaneGeometry groundGeometry(50.0, 5.0);            // 地面
  PlaneGeometry grassGeometry(1.0, 1.0);               // 草丛
  BoxGeometry containerGeometry(1.0, 1.0, 1.0);        // 箱子
  BoxGeometry skyboxGeometry(1.0, 1.0, 1.0);           // 天空盒
  SphereGeometry pointLightGeometry(0.04, 10.0, 10.0); // 点光源位置显示

  unsigned int woodMap = loadTexture("./static/texture/wall.jpg");                         // 地面
  unsigned int brickMap = loadTexture("./static/texture/brick_diffuse.jpg");               // 砖块
  unsigned int grassMap = loadTexture("./static/texture/blending_transparent_window.png"); // 草丛

  float factor = 0.0;

  // 旋转矩阵
  glm::mat4 ex = glm::eulerAngleX(45.0f);
  glm::mat4 ey = glm::eulerAngleY(45.0f);
  glm::mat4 ez = glm::eulerAngleZ(45.0f);

  glm::mat4 qularXYZ = glm::eulerAngleXYZ(45.0f, 45.0f, 45.0f);

  float fov = 45.0f; // 视锥体的角度
  glm::vec3 view_translate = glm::vec3(0.0, 0.0, -5.0);
  ImVec4 clear_color = ImVec4(25.0 / 255.0, 25.0 / 255.0, 25.0 / 255.0, 1.0); // 25, 25, 25

  glm::vec3 lightPosition = glm::vec3(1.0, 2.5, 2.0); // 光照位置

  // 设置平行光光照属性
  sceneShader.setVec3("directionLight.ambient", 0.6f, 0.6f, 0.6f);
  sceneShader.setVec3("directionLight.diffuse", 0.9f, 0.9f, 0.9f); // 将光照调暗了一些以搭配场景
  sceneShader.setVec3("directionLight.specular", 1.0f, 1.0f, 1.0f);

  // 设置衰减
  sceneShader.setFloat("light.constant", 1.0f);
  sceneShader.setFloat("light.linear", 0.09f);
  sceneShader.setFloat("light.quadratic", 0.032f);

  // 点光源的位置
  glm::vec3 pointLightPositions[] = {
      glm::vec3(0.7f, 1.0f, 1.5f),
      glm::vec3(2.3f, 3.0f, -4.0f),
      glm::vec3(-4.0f, 2.0f, 1.0f),
      glm::vec3(1.4f, 2.0f, 1.3f)};
  // 点光源颜色
  glm::vec3 pointLightColors[] = {
      glm::vec3(1.0f, 0.0f, 0.0f),
      glm::vec3(1.0f, 0.0f, 1.0f),
      glm::vec3(0.0f, 0.0f, 1.0f),
      glm::vec3(0.0f, 1.0f, 0.0f)};

  // 草的位置
  vector<glm::vec3> grassPositions{
      glm::vec3(-1.5f, 0.5f, -0.48f),
      glm::vec3(1.5f, 0.5f, 0.51f),
      glm::vec3(0.0f, 0.5f, 0.7f),
      glm::vec3(-0.3f, 0.5f, -2.3f),
      glm::vec3(0.5f, 0.5f, -0.6f)};

  // 天空盒贴图
  vector<string> faces{
      "./static/texture/Park3Med/px.jpg",
      "./static/texture/Park3Med/nx.jpg",
      "./static/texture/Park3Med/py.jpg",
      "./static/texture/Park3Med/ny.jpg",
      "./static/texture/Park3Med/pz.jpg",
      "./static/texture/Park3Med/nz.jpg"};

  unsigned int cubemapTexture = loadCubemap(faces);

  while (!glfwWindowShouldClose(window))
  {
    processInput(window);

    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastTime;
    lastTime = currentFrame;



    // 跳跃逻辑
    if (camera.IsJumping)
    {
      float elapsed = currentFrame - jumpStartTime; // 当前经过时间
      if (elapsed <= jumpDuration)
      {
          // 线性插值计算当前高度
          float t = elapsed / jumpDuration; // 归一化时间 [0, 1]
          float heightOffset = maxJumpHeight * (1.0f - fabs(1.0f - 2.0f * t)); // 抛物线运动

          // 更新摄像机高度
          camera.UpdateHeight(camera.BaseHeight + heightOffset);
      }
      else
      {
          // 跳跃结束，回到初始高度
          camera.EndJump();
      }
    }






    // 在标题中显示帧率信息
    // *************************************************************************
    int fps_value = (int)round(ImGui::GetIO().Framerate);
    int ms_value = (int)round(1000.0f / ImGui::GetIO().Framerate);

    std::string FPS = std::to_string(fps_value);
    std::string ms = std::to_string(ms_value);
    std::string newTitle = "LearnOpenGL - " + ms + " ms/frame " + FPS;
    glfwSetWindowTitle(window, newTitle.c_str());

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    // *************************************************************************

    // 渲染指令
    // ...
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 绘制天空盒
    drawSkyBox(skyboxShader, skyboxGeometry, cubemapTexture);

    // 修改光源颜色
    glm::vec3 lightColor;
    lightColor.x = sin(glfwGetTime() * 2.0f);
    lightColor.y = sin(glfwGetTime() * 0.7f);
    lightColor.z = sin(glfwGetTime() * 1.3f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, woodMap);

    float radius = 5.0f;
    float camX = sin(glfwGetTime() * 0.5) * radius;
    float camZ = cos(glfwGetTime() * 0.5) * radius;

    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(fov), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);

    glm::vec3 lightPos = glm::vec3(lightPosition.x * glm::sin(glfwGetTime()) * 2.0, lightPosition.y, lightPosition.z);

    // 绘制天空盒
    // *****************************************
    // glDepthFunc(GL_LEQUAL);
    // glDisable(GL_DEPTH_TEST);

    // skyboxShader.use();
    // view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // 移除平移分量

    // skyboxShader.setMat4("view", view);
    // skyboxShader.setMat4("projection", projection);

    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    // glBindVertexArray(skyboxGeometry.VAO);
    // glDrawElements(GL_TRIANGLES, skyboxGeometry.indices.size(), GL_UNSIGNED_INT, 0);

    // glBindVertexArray(0);
    // glDepthFunc(GL_LESS);
    // glEnable(GL_DEPTH_TEST);
    // view = camera.GetViewMatrix();

    // *****************************************

    sceneShader.use();
    factor = glfwGetTime();
    sceneShader.setFloat("factor", -factor * 0.3);
    sceneShader.setMat4("view", view);
    sceneShader.setMat4("projection", projection);

    sceneShader.setVec3("directionLight.direction", lightPos); // 光源位置
    sceneShader.setVec3("viewPos", camera.Position);

    pointLightPositions[0].z = camZ;
    pointLightPositions[0].x = camX;

    for (unsigned int i = 0; i < 4; i++)
    {

      // 设置点光源属性
      sceneShader.setVec3("pointLights[" + std::to_string(i) + "].position", pointLightPositions[i]);
      sceneShader.setVec3("pointLights[" + std::to_string(i) + "].ambient", 0.01f, 0.01f, 0.01f);
      sceneShader.setVec3("pointLights[" + std::to_string(i) + "].diffuse", pointLightColors[i]);
      sceneShader.setVec3("pointLights[" + std::to_string(i) + "].specular", 1.0f, 1.0f, 1.0f);

      // // 设置衰减
      sceneShader.setFloat("pointLights[" + std::to_string(i) + "].constant", 1.0f);
      sceneShader.setFloat("pointLights[" + std::to_string(i) + "].linear", 0.09f);
      sceneShader.setFloat("pointLights[" + std::to_string(i) + "].quadratic", 0.032f);
    }

    // 绘制地板
    // ********************************************************
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0, 0.0, 1.0));  // 再绕 Y 轴旋转 90 度
    
    // 向摄像机方向延伸地面
    model = glm::translate(model, glm::vec3(-5.0, 0.0, 0.0));  // 沿摄像机方向平移

    sceneShader.setFloat("uvScale", 4.0f);
    sceneShader.setMat4("model", model);

    glBindVertexArray(groundGeometry.VAO);
    glDrawElements(GL_TRIANGLES, groundGeometry.indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    // ********************************************************

    // 左路沿
    glm::mat4 leftCurbModel = glm::mat4(1.0f);
    leftCurbModel = glm::rotate(leftCurbModel, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
    leftCurbModel = glm::rotate(leftCurbModel, glm::radians(90.0f), glm::vec3(0.0, 0.0, 1.0));
    leftCurbModel = glm::translate(leftCurbModel, glm::vec3(5.0, 0.8 / 2, 0.0));
    leftCurbModel = glm::scale(leftCurbModel, glm::vec3(50.0, 0.8, 0.5));

    sceneShader.setFloat("uvScale", 1.0f);
    sceneShader.setMat4("model", leftCurbModel);

    glBindVertexArray(containerGeometry.VAO);
    glDrawElements(GL_TRIANGLES, containerGeometry.indices.size(), GL_UNSIGNED_INT, 0);

    // 右路沿
    glm::mat4 rightCurbModel = glm::mat4(1.0f);
    rightCurbModel = glm::rotate(rightCurbModel, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
    rightCurbModel = glm::rotate(rightCurbModel, glm::radians(90.0f), glm::vec3(0.0, 0.0, 1.0));
    rightCurbModel = glm::translate(rightCurbModel, glm::vec3(-5.0, 0.8 / 2, 0.0));
    rightCurbModel = glm::scale(rightCurbModel, glm::vec3(50.0, 0.8, 0.5));

    sceneShader.setFloat("uvScale", 1.0f);
    sceneShader.setMat4("model", rightCurbModel);

    glBindVertexArray(containerGeometry.VAO);
    glDrawElements(GL_TRIANGLES, containerGeometry.indices.size(), GL_UNSIGNED_INT, 0);


    // 绘制砖块
    // ----------------------------------------------------------
    glBindTexture(GL_TEXTURE_2D, brickMap);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(1.0, 1.0, -1.0));
    model = glm::scale(model, glm::vec3(2.0, 2.0, 2.0));

    sceneShader.setFloat("uvScale", 1.0f);
    sceneShader.setMat4("model", model);

    glBindVertexArray(containerGeometry.VAO);
    glDrawElements(GL_TRIANGLES, containerGeometry.indices.size(), GL_UNSIGNED_INT, 0);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-1.0, 0.5, 2.0));
    sceneShader.setMat4("model", model);

    glBindVertexArray(containerGeometry.VAO);
    glDrawElements(GL_TRIANGLES, containerGeometry.indices.size(), GL_UNSIGNED_INT, 0);
    // ----------------------------------------------------------

    // 绘制草丛面板
    // ----------------------------------------------------------
    glBindVertexArray(grassGeometry.VAO);
    glBindTexture(GL_TEXTURE_2D, grassMap);

    // 对透明物体进行动态排序
    std::map<float, glm::vec3> sorted;
    for (unsigned int i = 0; i < grassPositions.size(); i++)
    {
      float distance = glm::length(camera.Position - grassPositions[i]);
      sorted[distance] = grassPositions[i];
    }

    for (std::map<float, glm::vec3>::reverse_iterator iterator = sorted.rbegin(); iterator != sorted.rend(); iterator++)
    {
      model = glm::mat4(1.0f);
      model = glm::translate(model, iterator->second);
      sceneShader.setMat4("model", model);
      glDrawElements(GL_TRIANGLES, grassGeometry.indices.size(), GL_UNSIGNED_INT, 0);
    }
    // ----------------------------------------------------------

    // 绘制灯光物体
    // ************************************************************
    lightObjectShader.use();
    lightObjectShader.setMat4("view", view);
    lightObjectShader.setMat4("projection", projection);

    model = glm::mat4(1.0f);
    model = glm::translate(model, lightPos);

    lightObjectShader.setMat4("model", model);
    lightObjectShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));

    glBindVertexArray(pointLightGeometry.VAO);
    glDrawElements(GL_TRIANGLES, pointLightGeometry.indices.size(), GL_UNSIGNED_INT, 0);

    for (unsigned int i = 0; i < 4; i++)
    {
      model = glm::mat4(1.0f);
      model = glm::translate(model, pointLightPositions[i]);

      lightObjectShader.setMat4("model", model);
      lightObjectShader.setVec3("lightColor", pointLightColors[i]);

      glBindVertexArray(pointLightGeometry.VAO);
      glDrawElements(GL_TRIANGLES, pointLightGeometry.indices.size(), GL_UNSIGNED_INT, 0);
    }
    // ************************************************************

    // 渲染 gui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  containerGeometry.dispose();
  skyboxGeometry.dispose();
  groundGeometry.dispose();
  pointLightGeometry.dispose();
  glfwTerminate();

  return 0;
}

// 窗口变动监听
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
  glViewport(0, 0, width, height);
}

// 键盘输入监听
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // 跳跃时也允许移动
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);


    // 空格触发跳跃
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && camera.CanJump())
    {
        camera.StartJump();
        jumpStartTime = glfwGetTime(); // 记录跳跃开始时间
    }
}

// 鼠标移动监听
void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    static bool firstMouse = true; // 标记是否是第一次移动鼠标
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // 注意这里Y方向的计算

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);

    // std::cout << "Pitch: " << camera.Pitch << ", Yaw: " << camera.Yaw << std::endl; // 调试信息
}

// 加载纹理贴图
unsigned int loadTexture(char const *path)
{
  unsigned int textureID;
  glGenTextures(1, &textureID);

  // 图像y轴翻转
  stbi_set_flip_vertically_on_load(true);
  int width, height, nrComponents;
  unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
  if (data)
  {
    GLenum format;
    if (nrComponents == 1)
      format = GL_RED;
    else if (nrComponents == 3)
      format = GL_RGB;
    else if (nrComponents == 4)
      format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  }
  else
  {
    std::cout << "Texture failed to load at path: " << path << std::endl;
    stbi_image_free(data);
  }

  return textureID;
}

// 加载立方体贴图

unsigned int loadCubemap(vector<std::string> faces)
{
  unsigned int textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

  // 此处需要将y轴旋转关闭，若之前调用过loadTexture
  stbi_set_flip_vertically_on_load(false);

  int width, height, nrChannels;
  for (unsigned int i = 0; i < faces.size(); i++)
  {
    unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                   0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
      stbi_image_free(data);
    }
    else
    {
      std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
      stbi_image_free(data);
    }
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  return textureID;
}

// *******************method*********************
void drawSkyBox(Shader shader, BoxGeometry geometry, unsigned int cubeMap)
{

  glDepthFunc(GL_LEQUAL);
  glDisable(GL_DEPTH_TEST);

  glm::mat4 view = camera.GetViewMatrix();
  glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);

  shader.use();
  view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // 移除平移分量

  shader.setMat4("view", view);
  shader.setMat4("projection", projection);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);
  glBindVertexArray(geometry.VAO);
  glDrawElements(GL_TRIANGLES, geometry.indices.size(), GL_UNSIGNED_INT, 0);

  glBindVertexArray(0);
  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);
  view = camera.GetViewMatrix();
}