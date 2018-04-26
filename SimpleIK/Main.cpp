
// GLEW
#include <GL/glew.h>

// GLFW
#include "GLFW/glfw3.h"

#include "Shader.h"
#include "IK.h"
#include "MathHelper.h"

#define NUMBER_OF_JOINTS 7

Skeleton skel;

// Time tracker
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

double mouseX, mouseY;
glm::vec2 targetPosition;

bool bPlayWithAnimation = true;
bool bPlaying = false;

int width, height;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_A)
	{
		if (action == GLFW_RELEASE)
		{
			bPlayWithAnimation = !bPlayWithAnimation;
		}
	}
}

void mouseKey_callback(GLFWwindow* window, int key, int action, int mode)
{
	if (key == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (action == GLFW_RELEASE)
		{
			targetPosition.x = mouseX;
			targetPosition.y = height - mouseY;
			if (!bPlayWithAnimation)
			{
				IKSolver(skel, targetPosition);
			}
			else
			{
				bPlaying = true;
			}
		}
	}
}

void mouse_callback(GLFWwindow* window, double xPos, double yPos)
{
	mouseX = xPos;
	mouseY = yPos;
}

int main(int argc, char ** argv)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// Create Window
	GLFWwindow* window = glfwCreateWindow(1280, 720, "IK Solver Homework", nullptr, nullptr);
	if (window == nullptr)
	{
		glfwTerminate();
		std::cout << "Failed to create GLFW window" << std::endl;
		return -1;
	}
	glfwMakeContextCurrent(window);

	// init glew
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		glfwTerminate();
		std::cout << "Failed to init GLEW: "  << glewGetErrorString(err) << std::endl;
		return -1;
	}

	// viewport setup
	glfwGetFramebufferSize(window, &width, &height);

	// init skeleton
	if (argc > 1)
	{
		readSkeletonSetup(argv[1], skel);
	}
	else
	{
		setupSkeleton(skel, NUMBER_OF_JOINTS);
	}

	skel.m_position = glm::vec2(width * 0.5, height);

	// Init shaders
	Shader shader("SimpleIKShader.vs", "SimpleIKShader.frag");

	// Setup segment buffer
	float segmentData[] =
	{
		15.0f, 0.0f,
		-15.0f, 0.0f,
		0.0f, -100.0f
	};

	unsigned int segmentBuffer;
	glGenBuffers(1, &segmentBuffer);

	unsigned int segmentVAO;
	glGenVertexArrays(1, &segmentVAO);

	glBindVertexArray(segmentVAO);
	glBindBuffer(GL_ARRAY_BUFFER, segmentBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(segmentData), segmentData, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Init callbacks
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouseKey_callback);

	glViewport(0, 0, width, height);

	glm::mat3x3 skelMats[NUMBER_OF_JOINTS];

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		// calculate delta time
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		if (bPlayWithAnimation && bPlaying)
		{
			IKSolverStep(skel, targetPosition, 20.0f * deltaTime);
		}

		// Calculate/Update world position
		glm::mat3x3 worldMatrix;
		worldMatrix = rotate(worldMatrix, skel.m_rotation);
		worldMatrix = translate(worldMatrix, skel.m_position);

		{
			glm::mat3x3 model;
			model = rotate(model, skel.m_joints[0].m_rotation);
			model = translate(model, skel.m_joints[0].m_position);

			worldMatrix = worldMatrix * model;
			skelMats[0] = worldMatrix;
		}

		for (unsigned int i = 1; i < skel.m_jointCount; i++)
		{
			glm::mat3x3 model;
			model = rotate(model, skel.m_joints[i].m_rotation);
			model = translate(model, skel.m_joints[i].m_position);

			skelMats[i] = skelMats[ skel.m_joints[i].m_parent ] * model;
		}

		// Clear Screen
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Draw
		shader.Use();
		shader.setFloat("width", width);
		shader.setFloat("height", height);

		glBindVertexArray(segmentVAO);

		for (unsigned int i = 0; i < skel.m_jointCount-1; i++)
		{
			glm::mat3x3 scaleMatrix;
			if(i<skel.m_jointCount)
				scaleMatrix = scale(scaleMatrix, glm::vec2(1.0f, skel.m_joints[i+1].m_position.y / -100.0f));
			shader.setMat3("modelMatrix", skelMats[i] * scaleMatrix);

			glDrawArrays(GL_TRIANGLES, 0, 3);
		}

		glBindVertexArray(0);

		// Swap Buffers
		glfwSwapBuffers(window);
	}

	clearSkeleton(skel);

	glDeleteVertexArrays(1, &segmentVAO);
	glDeleteBuffers(1, &segmentBuffer);

	glfwTerminate();

	return 0;
}