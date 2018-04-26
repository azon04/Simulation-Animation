#pragma once

#include "glm/glm.hpp"

#define PI 3.1415926535f

#define DegToRad(value) ((value) / 180.0 * PI)

glm::mat3x3 scale(glm::mat3x3 mat, glm::vec2 scaleFactor);
glm::mat3x3 rotate(glm::mat3x3 mat, float rotInDeg);
glm::mat3x3 translate(glm::mat3x3 mat, glm::vec2 offset);