#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.05f;
const float ZOOM = 45.0f;

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
	// camera Attributes
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	glm::vec3 StartPosition; // 摄像机的起始位置
	// euler Angles
	float Yaw;
	float Pitch;
	// camera options
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;
	bool LimitYaw; // 是否限制偏航角
	// Jump-related variables
	float BaseHeight = 0.0f; // 初始高度（XZ平面高度）
    float CurrentHeight = 0.0f; // 当前高度
    bool IsJumping = false; // 是否正在跳跃
	bool IsDescending = false; // 是否正在下降

	// Auto-move variables
	static bool IsInitialPhase; // 是否处于初始阶段
    static bool IsAutoMoving; // 是否正在自动移动
    static float AutoMoveStartTime; // 自动移动的开始时间
	static float AutoMoveDuration; // 自动移动的持续时间
	static constexpr float MaxMoveDistance = 45.0f;  // 道路长度

	static std::vector<glm::vec3> detectedGrassPositions; // 草丛位置

	// Constructor
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = YAW, float pitch = PITCH);

    glm::mat4 GetViewMatrix();

    void ProcessKeyboard(Camera_Movement direction, float deltaTime, bool autoMove = false);
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
    void ProcessMouseScroll(float yoffset);

    void SetBaseHeight(float height);
    void UpdateHeight(float height);
    bool CanJump();
    void StartJump();
    void EndJump();

	void UpdateAutoMove(float deltaTime, float currentTime, const std::vector<glm::vec3>& grassPositions);

	void StartAutoMove(float startTime); // 开始自动移动
    void StopAutoMove();                // 停止自动移动
	void ResetToStartPosition(); // 重置到起始位置

	static bool GameOver;
	static bool GameSuccess;
private:
    void updateCameraVectors();
};

#endif