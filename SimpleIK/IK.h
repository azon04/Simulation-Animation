#pragma once


#include "glm/glm.hpp"

struct Joint
{
	glm::vec2 m_position;
	float m_rotation;
	unsigned int m_parent;

	// Constraint
	float m_minRotation;
	float m_maxRotation;

	// World position update every frame
	glm::vec2 m_worldPosition;
	glm::mat3 m_matrix;
};

struct Skeleton
{
	glm::vec2 m_position;
	float m_rotation;

	unsigned int m_jointCount;
	Joint* m_joints;
};

void setupSkeleton(Skeleton& skeleton, int numberOfJoint);
void clearSkeleton(Skeleton& skeleton);
void readSkeletonSetup(const char* filename, Skeleton& skeleton);

void IKSolver(Skeleton& skeleton, glm::vec2 target);
void IKSolverStep(Skeleton& skeleton, glm::vec2 target, float step);

void CCDSolver(Skeleton& skeleton, glm::vec2& target);
void CCDSolverStep(Skeleton& skeleton, glm::vec2& target, float step);
