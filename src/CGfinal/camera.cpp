#include "camera.h"
#include <GLFW/glfw3.h> // 添加 GLFW 头文件

// 静态变量定义
bool Camera::IsAutoMoving = false; 
float Camera::AutoMoveStartTime = 0.0f;
bool Camera::IsInitialPhase = true; // 默认为初始阶段
float Camera::AutoMoveDuration = 5.0f; // 修改默认持续时间
bool Camera::GameOver = false;
bool Camera::GameSuccess = false;

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED),
      MouseSensitivity(SENSITIVITY), Zoom(ZOOM), BaseHeight(position.y),
      LimitYaw(true) // 初始状态下限制偏航角
{
    Position = position;
    StartPosition = position; // 初始化起始位置
    Position.y = BaseHeight; // 确保 Position.y 和 BaseHeight 一致
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix()
{
    return glm::lookAt(Position, Position + Front, Up);
}

void Camera::SetBaseHeight(float height)
{
    BaseHeight = height;
    Position.y = BaseHeight;
}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime, bool autoMove)
{
    float velocity = MovementSpeed * deltaTime;

    glm::vec3 moveDir(0.0f);

    // 自动移动时，仅允许左右移动
    if (autoMove)
    {
        if (direction == LEFT)
            moveDir -= glm::normalize(glm::cross(Front, Up)) * velocity;
        if (direction == RIGHT)
            moveDir += glm::normalize(glm::cross(Front, Up)) * velocity;

        Position += moveDir; // 仅更新左右位移
        return;
    }

    // 普通模式下的移动逻辑
    if (direction == FORWARD)
        moveDir += glm::vec3(Front.x, 0.0f, Front.z) * velocity;
    if (direction == BACKWARD)
        moveDir -= glm::vec3(Front.x, 0.0f, Front.z) * velocity;
    if (direction == LEFT)
        moveDir -= glm::normalize(glm::cross(Front, Up)) * velocity;
    if (direction == RIGHT)
        moveDir += glm::normalize(glm::cross(Front, Up)) * velocity;

    Position += moveDir;    // 只更新水平位移，不修改高度
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
{
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    if (constrainPitch)
    {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    // 限制偏航角范围为 [-90, 90]
    if (LimitYaw) // 如果限制偏航角范围
    {
        if (Yaw > -45.0f)  // 限制最大右偏航角
            Yaw = -45.0f;
        if (Yaw < -135.0f) // 限制最大左偏航角
            Yaw = -135.0f;
    }

        
    updateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset)
{
    Zoom -= (float)yoffset;
    if (Zoom < 1.0f)
        Zoom = 1.0f;
    if (Zoom > 45.0f)
        Zoom = 45.0f;
}


void Camera::UpdateHeight(float height)
{
    CurrentHeight = height;
    Position.y = CurrentHeight; // 实时更新 Y 高度
}

bool Camera::CanJump()
{
    return !IsJumping;
}

void Camera::StartJump()
{
    IsJumping = true;
}

void Camera::EndJump()
{
    IsJumping = false;
    CurrentHeight = BaseHeight; // 重置高度
    Position.y = BaseHeight;
}

void Camera::UpdateAutoMove(float deltaTime, float currentTime, const std::vector<glm::vec3>& grassPositions)
{
    if (!IsAutoMoving) return;

    // 检测碰撞
    for (const auto& grassPosition : grassPositions) {
        // 检查 z 坐标接近，x 在范围内，并且未跳跃和未下降
        if (fabs(Position.z - grassPosition.z) < 0.1f &&
            Position.x >= grassPosition.x - 0.3f && Position.x <= grassPosition.x + 0.3f &&
            !IsJumping && !IsDescending)
        {
            IsAutoMoving = false; // 停止自动移动
            GameOver = true;     // 触发游戏结束
            return;
        }
    }

    // 自动移动

    float velocity = MaxMoveDistance / AutoMoveDuration * deltaTime; // 固定速度
    Position.z -= velocity; // 沿 z 轴负方向移动

    // 计算自动移动的时间
    float elapsedTime = currentTime - AutoMoveStartTime;

    // 如果超出最大移动时间，则停止自动移动
    if (elapsedTime >= AutoMoveDuration)
    {
        StopAutoMove();
        GameSuccess = true;
        return;
    }


}

void Camera::updateCameraVectors()
{
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);

    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}

void Camera::StartAutoMove(float startTime)
{
    IsAutoMoving = true;
    LimitYaw = true; // 自动移动时限制偏航角
    AutoMoveStartTime = startTime; // 记录开始时间
}

void Camera::StopAutoMove()
{
    IsAutoMoving = false;
    LimitYaw = false; // 停止自动移动时解除偏航角限制
    // 清除偏航角限制的状态
    updateCameraVectors();
}

void Camera::ResetToStartPosition() {
    Position = StartPosition;
    Position.y = BaseHeight; // 确保高度一致
    updateCameraVectors();   // 更新方向向量
}