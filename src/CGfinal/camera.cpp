#include "camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED),
      MouseSensitivity(SENSITIVITY), Zoom(ZOOM), BaseHeight(position.y)
{
    Position = position;
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

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
    float velocity = MovementSpeed * deltaTime;
    glm::vec3 moveDir(0.0f);

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
