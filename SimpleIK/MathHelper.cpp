#include "MathHelper.h"

glm::mat3x3 scale(glm::mat3x3 mat, glm::vec2 scaleFactor)
{
	glm::mat3x3 factorMatrix;
	factorMatrix[0].x = scaleFactor.x;
	factorMatrix[1].y = scaleFactor.y;

	return factorMatrix * mat;
}

glm::mat3x3 rotate(glm::mat3x3 mat, float rotInDeg)
{
	glm::mat3x3 rotMat;
	float sinA = sin(rotInDeg / 180.0f * PI);
	float cosA = cos(rotInDeg / 180.0f * PI);

	rotMat[0].x = cosA;
	rotMat[0].y = -sinA;

	rotMat[1].x = sinA;
	rotMat[1].y = cosA;

	return rotMat * mat;
}

glm::mat3x3 translate(glm::mat3x3 mat, glm::vec2 offset)
{
	mat[2].x = offset.x;
	mat[2].y = offset.y;

	return mat;
}
