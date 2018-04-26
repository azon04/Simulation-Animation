#pragma once

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum CameraMovement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

// Default camera values
const GLfloat YAW = -90.0f;
const GLfloat PITCH = 0.0f;
const GLfloat SPEED = 3.0f;
const GLfloat SENSITIVITY = 0.25f;
const GLfloat ZOOM = 45.0f;


class Camera
{
public:
	// Camera atrributes
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;

	// Euler Angles
	GLfloat Yaw;
	GLfloat Pitch;

	// Camera Options
	GLfloat MovementSpeed;
	GLfloat MouseSensitiviy;
	GLfloat Zoom;

	// Constructor with vector
	Camera(glm::vec3 position, glm::vec3 up, GLfloat yaw = YAW, GLfloat pitch = PITCH);

	// Constructor with scalar value
	Camera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat upX, GLfloat upY, GLfloat upZ, GLfloat yaw = YAW, GLfloat pitch = PITCH);

	~Camera();

	glm::mat4 GetViewMatrix();

	void ProcessKeyboard(CameraMovement direction, GLfloat deltaTime);

	void ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constraintPitch = true);

	void ProcessMouseScroll(GLfloat yoffset);

private:

	void UpdateCameraVectors();
};

