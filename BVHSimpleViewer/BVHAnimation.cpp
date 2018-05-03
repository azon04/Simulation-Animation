#include "BVHAnimation.h"

#include <stdio.h>
#include <iostream>

// GLEW
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

#define StringToValue(_var, _string, value) \
	if(strcmp(_string,_var) == 0) \
	{ \
		return value; \
	}

#define PI 3.1415926535f

BVHAnimation::~BVHAnimation()
{
	glDeleteVertexArrays(1, &m_skeletonVAO);
	glDeleteBuffers(1, &m_jointsVBO);
}

void BVHAnimation::readFile(const char * _fileName)
{
	FILE* file = fopen(_fileName, "r");

	if (file == NULL)
	{
		std::cout << "Can't open file: " << _fileName << std::endl;
		exit(1);
	}

	char buffer[1024];

	// clear motion data
	m_motionData.m_numberOfChannel = 0;
	m_motionData.m_motionData.clear();

	// clear skeleton
	m_skeleton.m_jointCount = 0;
	m_skeleton.m_joints.clear();

	// read HIERARCHY
	fscanf(file, "%s\n", buffer);

	// read root
	fscanf(file, "%s", buffer);

	m_skeleton.m_joints.push_back(Joint());
	Joint& rootJoint = m_skeleton.m_joints[0];
	rootJoint.m_parent = -1;
	rootJoint.m_jointIndex = 0;
	processJoint(file, 0);

	m_skeleton.m_jointCount = m_skeleton.m_joints.size();

	// count 
	// read MOTION
	fscanf(file, "%s\n", buffer);

	// read frames:
	fscanf(file, "%s", buffer);
	fscanf(file, "%d\n", &m_motionData.m_frames);

	// read Frame Times;
	fscanf(file, "%s", buffer);
	fscanf(file, "%s", buffer);
	fscanf(file, "%f\n", &m_motionData.m_frameTime);

	for (unsigned int i_frame = 0; i_frame < m_motionData.m_frames; i_frame++)
	{
		for (unsigned int i = 0; i < m_motionData.m_numberOfChannel; i++)
		{
			float value;
			fscanf(file, "%f", &value);
			m_motionData.m_motionData.push_back(value);
		}
	}
	fclose(file);


	// setup matrices
	m_matrices.resize(m_skeleton.m_jointCount);

	m_bufferStream.clear();

	// calculating stream
	for (int i = 0; i < m_skeleton.m_jointCount; i++)
	{
		JointBufferData bufferData1, bufferData2;
		bufferData1.m_position = glm::vec3(0.0f, 0.0f, 0.0f);
		bufferData2.m_position = m_skeleton.m_joints[i].m_position;
		bufferData2.m_index = bufferData1.m_index = i;

		m_bufferStream.push_back(bufferData1);
		m_bufferStream.push_back(bufferData2);
	}

	glGenVertexArrays(1, &m_skeletonVAO);
	glGenBuffers(1, &m_jointsVBO);

	glBindVertexArray(m_skeletonVAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_jointsVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * m_bufferStream.size(), &m_bufferStream[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float) + 1 * sizeof(GLint), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float) + 1 * sizeof(GLint), (void*) (3*sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

void BVHAnimation::processJoint(FILE* file, int joint_index)
{
	char buffer[32];

	fscanf(file, "%s", buffer);
	
	m_skeleton.m_joints[joint_index].m_jointName = buffer;

	fscanf(file, "%s", buffer); // Skip "{"

	// read OFFSET/CHANNEL/JOINT
	fscanf(file, "%s", buffer);
	while (strcmp(buffer, "}") != 0)
	{
		if (strcmp(buffer, "OFFSET") == 0)
		{
			fscanf(file, "%f %f %f\n", &m_skeleton.m_joints[joint_index].m_position.x, 
				&m_skeleton.m_joints[joint_index].m_position.y, 
				&m_skeleton.m_joints[joint_index].m_position.z);
		}
		else if (strcmp(buffer, "CHANNELS") == 0)
		{
			fscanf(file, "%d", &m_skeleton.m_joints[joint_index].m_channelCount);
			for (unsigned int i = 0; i < m_skeleton.m_joints[joint_index].m_channelCount; i++)
			{
				fscanf(file, "%s", &buffer);
				m_skeleton.m_joints[joint_index].m_channels.push_back(getChannelBasedOnString(buffer));
			}
			m_motionData.m_numberOfChannel += m_skeleton.m_joints[joint_index].m_channelCount;
		}
		else if (strcmp(buffer, "JOINT") == 0)
		{
			m_skeleton.m_joints.push_back(Joint());
			Joint& joint = m_skeleton.m_joints[m_skeleton.m_joints.size() - 1];
			joint.m_parent = joint_index;
			joint.m_jointIndex = m_skeleton.m_joints.size() - 1;
			processJoint(file, joint.m_jointIndex);
		}
		else if (strcmp(buffer, "End") == 0)
		{
			m_skeleton.m_joints.push_back(Joint());
			Joint& joint = m_skeleton.m_joints[m_skeleton.m_joints.size() - 1];
			joint.m_parent = joint_index;
			joint.m_jointIndex = m_skeleton.m_joints.size() - 1;
			processJoint(file, joint.m_jointIndex);
		}
		fscanf(file, "%s", buffer);
	}

}

BVH_Channel BVHAnimation::getChannelBasedOnString(const char* channelString)
{
	StringToValue(channelString, "Xposition", X_Position_Channel);
	StringToValue(channelString, "Yposition", Y_Position_Channel);
	StringToValue(channelString, "Zposition", Z_Position_Channel);
	StringToValue(channelString, "Xrotation", X_Rotation_Channel);
	StringToValue(channelString, "Yrotation", Y_Rotation_Channel);
	StringToValue(channelString, "Zrotation", Z_Rotation_Channel);
}

void BVHAnimation::update(float _deltaTime)
{
	m_currentTime += _deltaTime;
	m_currentFrame = (int)(m_currentTime / m_motionData.m_frameTime) % m_motionData.m_frames;
}

void BVHAnimation::draw(Shader* shader)
{
	// Calculating matrices
	glm::mat4 worldMatrix;

	worldMatrix = glm::scale(worldMatrix, glm::vec3(1.0f, 1.0f, 0.1f));
	worldMatrix = glm::rotate(worldMatrix, m_skeleton.m_rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
	worldMatrix = glm::rotate(worldMatrix, m_skeleton.m_rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
	worldMatrix = glm::rotate(worldMatrix, m_skeleton.m_rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
	worldMatrix = glm::translate(worldMatrix, m_skeleton.m_position);

	unsigned int channelIndex = m_motionData.m_numberOfChannel * m_currentFrame;
	float fraction = (m_currentTime / m_motionData.m_frameTime) - (int)(m_currentTime / m_motionData.m_frameTime);
	for (unsigned int i = 0; i < m_skeleton.m_jointCount; i++)
	{
		int parent = m_skeleton.m_joints[i].m_parent;
		glm::mat4 modelMatrix;

		for (unsigned int j = 0; j < m_skeleton.m_joints[i].m_channelCount; j++)
		{
			float value = m_motionData.m_motionData[channelIndex];
			// Interpolate value with next frame value
			if (m_currentFrame < m_motionData.m_frames - 1)
			{
				value = value * (1.0f - fraction) + m_motionData.m_motionData[channelIndex + m_motionData.m_numberOfChannel] * fraction;
			}
			modelMatrix = modelMatrix * getMatrixBasedOnChannel(m_skeleton.m_joints[i].m_channels[j], value);
			channelIndex++;
		}

		modelMatrix = glm::translate(glm::mat4(), m_skeleton.m_joints[i].m_position) * modelMatrix;

		if (parent >= 0)
		{
			m_matrices[i] = m_matrices[parent] * modelMatrix;
		}
		else
		{
			m_matrices[i] = worldMatrix * modelMatrix;
		}
	}


	// pass matrices into shader
	shader->setMat4("modelMat[0]", worldMatrix);
	for (unsigned int i = 1; i < m_skeleton.m_jointCount; i++)
	{
		char buffer[32];
		sprintf(buffer, "modelMat[%d]", (i));
		shader->setMat4(buffer, m_matrices[m_skeleton.m_joints[i].m_parent]);
	}

	// Call draw
	glBindVertexArray(m_skeletonVAO);
	glDrawArrays(GL_LINES, 0, m_bufferStream.size());
	glBindVertexArray(0);
}

glm::mat4 getMatrixBasedOnChannel(BVH_Channel channel, float value)
{
	glm::mat4 mat;
	switch (channel)
	{
	case X_Position_Channel:
		return glm::translate(mat, glm::vec3(value, 0.0f, 0.0f));
	case Y_Position_Channel:
		return glm::translate(mat, glm::vec3(0.0f, value, 0.0f));
	case Z_Position_Channel:
		return glm::translate(mat, glm::vec3(0.0f, 0.0f, value));
	case X_Rotation_Channel:
		return glm::rotate(mat, value * PI  / 180.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	case Y_Rotation_Channel:
		return glm::rotate(mat, value * PI / 180.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	case Z_Rotation_Channel:
		return glm::rotate(mat, value * PI / 180.0f, glm::vec3(0.0f, 0.0f, 1.0f));
	default:
		return mat;
	}
}
