#include "Camera.h"





Camera::Camera(glm::vec3 position, glm::vec3 up, GLfloat yaw, GLfloat pitch)
	: Position(position), WorldUp(up), Front(glm::vec3(0.0f,0.0f,-1.0f)), MovementSpeed(SPEED),
	MouseSensitiviy(SENSITIVITY), Zoom(ZOOM), Yaw(yaw), Pitch(pitch)
{
	UpdateCameraVectors();
}

Camera::Camera(GLfloat posX, GLfloat posY, GLfloat posZ, GLfloat upX, GLfloat upY, GLfloat upZ, GLfloat yaw /*= YAW*/, GLfloat pitch /*= PITCH*/)
	: Position(glm::vec3(posX, posY, posZ)), WorldUp(glm::vec3(upX, upY, upZ)), Front(glm::vec3(0.0f,0.0f,-1.0f)),
	MovementSpeed(SPEED), MouseSensitiviy(SENSITIVITY), Zoom(ZOOM), Yaw(yaw), Pitch(pitch)
{
	UpdateCameraVectors();
}

Camera::~Camera()
{

}

glm::mat4 Camera::GetViewMatrix()
{
	glm::vec3 zaxis = -Front;

	glm::vec3 xaxis = glm::normalize(glm::cross(glm::normalize(WorldUp), zaxis));
	glm::vec3 yaxis = glm::cross(zaxis, xaxis);

	// glm [col][row]
	glm::mat4 translation, rotation;
	translation[3][0] = -Position.x;
	translation[3][1] = -Position.y;
	translation[3][2] = -Position.z;

	rotation[0][0] = xaxis.x;
	rotation[1][0] = xaxis.y;
	rotation[2][0] = xaxis.z;
	
	rotation[0][1] = yaxis.x;
	rotation[1][1] = yaxis.y;
	rotation[2][1] = yaxis.z;

	rotation[0][2] = zaxis.x;
	rotation[1][2] = zaxis.y;
	rotation[2][2] = zaxis.z;

	return rotation * translation;
}

void Camera::ProcessKeyboard(CameraMovement direction, GLfloat deltaTime)
{
	GLfloat velocity = MovementSpeed * deltaTime;
	
	if (direction == FORWARD)
		this->Position += Front * velocity;
	
	if (direction == BACKWARD)
		this->Position -= Front * velocity;

	if (direction == RIGHT)
		this->Position += Right * velocity;

	if (direction == LEFT)
		this->Position -= Right * velocity;
}

void Camera::ProcessMouseMovement(GLfloat xoffset, GLfloat yoffset, GLboolean constraintPitch /*= true*/)
{
	xoffset *= MouseSensitiviy;
	yoffset *= MouseSensitiviy;

	this->Yaw += xoffset;
	this->Pitch += yoffset;

	if (constraintPitch) {
		this->Pitch = glm::clamp(Pitch, -89.0f, 89.0f);
	}

	// Update Front, Right and Up Vectors using the Updated Eular Angles
	this->UpdateCameraVectors();
}

void Camera::ProcessMouseScroll(GLfloat yoffset)
{
	Zoom -= yoffset * MouseSensitiviy;
	Zoom = glm::clamp(Zoom, 1.0f, 45.0f);
}

void Camera::UpdateCameraVectors()
{
	// Calculate the new Front Vector
	glm::vec3 front;
	front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	front.y = sin(glm::radians(Pitch));
	front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	this->Front = glm::normalize(front);
	// Also re calculate the Right and Up vector
	this->Right = glm::cross(Front, WorldUp);
	this->Up = glm::cross(Right, Front);
}
