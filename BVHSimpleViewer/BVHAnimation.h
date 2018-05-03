#pragma once

#include <vector>
#include <string>
#include "glm/glm.hpp"
#include "Shader.h"

enum BVH_Channel : unsigned int
{
	X_Position_Channel = 0,
	Y_Position_Channel = 1,
	Z_Position_Channel = 2,
	X_Rotation_Channel = 3,
	Y_Rotation_Channel = 4,
	Z_Rotation_Channel = 5
};

struct Joint
{
	glm::vec3 m_position;
	glm::vec3 m_rotation;
	unsigned int m_parent;
	unsigned int m_jointIndex;

	int m_channelCount;
	std::vector<BVH_Channel> m_channels;

	std::string m_jointName;
};

struct Skeleton
{
	glm::vec3 m_position;
	glm::vec3 m_rotation;

	unsigned int m_jointCount;
	std::vector<Joint> m_joints;
};

struct BVHMotionData
{
	int m_numberOfChannel;
	unsigned int m_frames;
	float m_frameTime;
	std::vector<float> m_motionData;
};

struct JointBufferData
{
	glm::vec3 m_position;
	GLint m_index;
};

class BVHAnimation
{
public:

	BVHAnimation():
		m_currentFrame(0),
		m_currentTime(0.0f)
	{

	}

	~BVHAnimation();

	Skeleton m_skeleton;
	BVHMotionData m_motionData;

	void readFile(const char * _fileName);
	void update(float _deltaTime);
	void draw(Shader* shader);
	
protected:
	void processJoint(FILE* file, int joint_index);

	BVH_Channel getChannelBasedOnString(const char* channelString);

	// this snap to lower frame animation
	// TODO interpolate for more accurate animation
	float m_currentTime;
	int m_currentFrame;

	// cache for matrices
	std::vector<glm::mat4> m_matrices;

	// cache for skeleton lines
	std::vector<JointBufferData> m_bufferStream;

	// for drawing
	unsigned int m_skeletonVAO;
	unsigned int m_jointsVBO;
	unsigned int m_indexVBO;
};

// Helper functions
glm::mat4 getMatrixBasedOnChannel(BVH_Channel channel, float value);

