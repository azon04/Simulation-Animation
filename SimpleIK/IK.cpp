#include "IK.h"
#include "MathHelper.h"

#include <iostream>
#include <stdio.h>
#include <cmath>

#define ACCEPTABLE_DISTANCE 2
#define MAX_NUMBER_ITERATION 60

#define MIN(A,B) ((A) < (B) ? (A) : (B))
#define MAX(A,B) ((A) > (B) ? (A) : (B))
#define CLAMP(V, minVal, maxVal) (MIN(MAX((V), (minVal)), (maxVal)))

void setupSkeleton(Skeleton& skeleton, int numberOfJoint)
{
	skeleton.m_jointCount = numberOfJoint;
	skeleton.m_joints = new Joint[numberOfJoint];

	for (unsigned int i = 0; i < skeleton.m_jointCount; i++)
	{
		if(i>0)
			skeleton.m_joints[i].m_position = glm::vec2(0.0f, -100.0f);
		skeleton.m_joints[i].m_rotation = 0.0f;
		skeleton.m_joints[i].m_parent = i - 1;
		skeleton.m_joints[i].m_minRotation = -60.0;
		skeleton.m_joints[i].m_maxRotation = 60.0;
	}
}

void clearSkeleton(Skeleton& skeleton)
{
	delete skeleton.m_joints;
}

void readSkeletonSetup(const char* filename, Skeleton& skeleton)
{
	FILE* file = fopen(filename, "r");

	if (file == NULL)
	{
		std::cout << "Can't open file: " << filename << std::endl;
		exit(1);
	}

	// Read Joint count
	fscanf(file, "%d\n", &skeleton.m_jointCount);
	skeleton.m_joints = new Joint[skeleton.m_jointCount];

	for (unsigned int i = 0; i < skeleton.m_jointCount; i++)
	{
		skeleton.m_joints[i].m_parent = i - 1;
		skeleton.m_joints[i].m_rotation = 0.0f;

		int id = 0;
		fscanf(file, "%d\n", &id);

		float length = 1.0f;
		fscanf(file, "%f\n", &length);
		skeleton.m_joints[i].m_position = glm::vec2(0.0f, -100.0f * length);

		fscanf(file, "%f %f\n", &skeleton.m_joints[i].m_minRotation, &skeleton.m_joints[i].m_maxRotation);
	}

	fclose(file);
}

void IKSolver(Skeleton& skeleton, glm::vec2 target)
{
	CCDSolver(skeleton, target);
}

void IKSolverStep(Skeleton& skeleton, glm::vec2 target, float step)
{
	CCDSolverStep(skeleton, target, step);
}

void updateWorldPosition(Skeleton& skel)
{
	glm::mat3x3 worldMatrix;
	worldMatrix = rotate(worldMatrix, skel.m_rotation);
	worldMatrix = translate(worldMatrix, skel.m_position);

	for (unsigned int i = 0; i < skel.m_jointCount; i++)
	{
		skel.m_joints[i].m_worldPosition = glm::vec2(worldMatrix * glm::vec3(skel.m_joints[i].m_position, 1.0f));
		
		glm::mat3x3 model;
		model = rotate(model, skel.m_joints[i].m_rotation);
		model = translate(model, skel.m_joints[i].m_position);

		worldMatrix = worldMatrix * model;
	}
}

void CCDSolver(Skeleton& skeleton, glm::vec2& target)
{
	updateWorldPosition(skeleton);

	glm::vec2 effector = skeleton.m_joints[skeleton.m_jointCount-1].m_worldPosition;
	float d = glm::dot(target - effector, target - effector);
	int iteration = 0;
	while ( d > (ACCEPTABLE_DISTANCE * ACCEPTABLE_DISTANCE) && iteration++ < MAX_NUMBER_ITERATION)
	{
		for (int i = skeleton.m_jointCount - 2; i >= 0; i--)
		{
			glm::vec2 JtoE = glm::normalize(effector - skeleton.m_joints[i].m_worldPosition);
			glm::vec2 JtoT = glm::normalize(target - skeleton.m_joints[i].m_worldPosition);

			float cosd = glm::dot(JtoT, JtoE);
			if (cosd >= 1.0f)
			{
				continue;
			}

			float theta = acos(cosd);
			float thetaInDeg = theta * 180 / PI;

			float z = JtoE.x * JtoT.y - JtoT.x * JtoE.y;

			float rotationBefore = skeleton.m_joints[i].m_rotation;
			skeleton.m_joints[i].m_rotation -= (z < 0 ? -1 : 1) * thetaInDeg;
			skeleton.m_joints[i].m_rotation = CLAMP(skeleton.m_joints[i].m_rotation, skeleton.m_joints[i].m_minRotation, skeleton.m_joints[i].m_maxRotation);
			thetaInDeg = skeleton.m_joints[i].m_rotation - rotationBefore;

			// Calculate new effector pos
			updateWorldPosition(skeleton);

			effector = skeleton.m_joints[skeleton.m_jointCount - 1].m_worldPosition;
			d = glm::dot(target - effector, target - effector);
		}

		
	}


}

void CCDSolverStep(Skeleton& skeleton, glm::vec2& target, float step)
{
	updateWorldPosition(skeleton);

	glm::vec2 effector = skeleton.m_joints[skeleton.m_jointCount - 1].m_worldPosition;
	float d = glm::dot(target - effector, target - effector);
	if (d > (ACCEPTABLE_DISTANCE * ACCEPTABLE_DISTANCE))
	{
		int alignCount = 0;
		for (int i = skeleton.m_jointCount - 2; i >= 0; i--)
		{
			glm::vec2 JtoE = glm::normalize(effector - skeleton.m_joints[i].m_worldPosition);
			glm::vec2 JtoT = glm::normalize(target - skeleton.m_joints[i].m_worldPosition);

			float cosd = glm::dot(JtoT, JtoE);
			if (cosd >= 0.9999f)
			{
				continue;
			}

			float theta = acos(cosd);
			float thetaInDeg = theta * 180 / PI;

			// Clamping thetaInDeg
			thetaInDeg = MIN(thetaInDeg, step);

			float z = JtoE.x * JtoT.y - JtoT.x * JtoE.y;

			float rotationBefore = skeleton.m_joints[i].m_rotation;
			skeleton.m_joints[i].m_rotation -= (z < 0 ? -1 : 1) * thetaInDeg;
			skeleton.m_joints[i].m_rotation = CLAMP(skeleton.m_joints[i].m_rotation, skeleton.m_joints[i].m_minRotation, skeleton.m_joints[i].m_maxRotation);
			thetaInDeg = skeleton.m_joints[i].m_rotation - rotationBefore;

			// Calculate new effector pos
			theta = thetaInDeg * PI / 180.0f;
			cosd = cos(theta);
			float sind = sin(-theta);

			effector -= skeleton.m_joints[i].m_worldPosition;

			float xNew = effector.x * cosd - effector.y * sind;
			float yNew = effector.x * sind + effector.y * cosd;

			effector = glm::vec2(xNew, yNew) + skeleton.m_joints[i].m_worldPosition;
		}
	}
}

