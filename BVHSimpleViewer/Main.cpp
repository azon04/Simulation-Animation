

// GLEW
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

#include "BVHAnimation.h"
#include "Shader.h"
#include "Camera.h"
#include "glm/glm.hpp"

// Time tracker
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// keep track key
bool keys[1024];

int width, height;
GLfloat lastX = 400, lastY = 300;

Camera camera(glm::vec3(0.0f, 20.0f, 45.0f), glm::vec3(0.0f, 1.0f, 0.0f));


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	// When user presses the escape key, we set the WindowsShouldClose property to true
	// closing the application
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE)
		keys[key] = false;
}

void mouseKey_callback(GLFWwindow* window, int key, int action, int mode)
{
	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE)
		keys[key] = false;
}

void mouse_callback(GLFWwindow* window, double xPos, double yPos)
{
	static bool firstMouse = true;
	if (keys[GLFW_MOUSE_BUTTON_RIGHT])
	{
		if (firstMouse) {
			firstMouse = false;
			lastX = xPos;
			lastY = yPos;
		}

		GLfloat xOffset = xPos - lastX;
		GLfloat yOffset = lastY - yPos;
		lastX = xPos;
		lastY = yPos;

		camera.ProcessMouseMovement(xOffset, yOffset);
	}
	else
	{
		firstMouse = true;
	}
}

void do_movement() {

	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(CameraMovement::FORWARD, deltaTime);

	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(CameraMovement::BACKWARD, deltaTime);

	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(CameraMovement::LEFT, deltaTime);

	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(CameraMovement::RIGHT, deltaTime);
}

int main(int argc, char ** argv)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// Create Window
	GLFWwindow* window = glfwCreateWindow(720, 720, "Simple BVH Viewer", nullptr, nullptr);
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
		std::cout << "Failed to init GLEW: " << glewGetErrorString(err) << std::endl;
		return -1;
	}

	// viewport setup
	glfwGetFramebufferSize(window, &width, &height);

	// Set up projection matrix
	glm::mat4 projMat = glm::perspective(glm::radians(45.0f), width / (float)height, 0.1f, 10000.0f);

	// Parse file
	if (argc < 2)
	{
		return -1;
	}

	BVHAnimation bvhAnimation;
	bvhAnimation.readFile(argv[1]);

	// Init shaders
	Shader lineShader("LineShader.vs", "LineShader.frag");
	Shader skelLineShader("SkeletonLineShader.vs", "LineShader.frag");

	// setup buffers
	float basisVectorsData[] =
	{
		// Pos,				Color
		0.0f, 0.0f, 0.0f,	1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,	1.0f, 0.0f, 0.0f,

		0.0f, 0.0f, 0.0f,	0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f,	0.0f, 0.0f, 1.0f,
		
		0.0f, 0.0f, 0.0f,	0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,	0.0f, 1.0f, 0.0f
	};

	unsigned int basisVAO;
	glGenVertexArrays(1, &basisVAO);

	unsigned int basisVBO;
	glGenBuffers(1, &basisVBO);

	glBindVertexArray(basisVAO);
	glBindBuffer(GL_ARRAY_BUFFER, basisVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(basisVectorsData), basisVectorsData, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	float floorVectorsData[] =
	{
		// Pos,		Color
		2.0f, 0.0f, 2.0f,	0.75f, 0.75f, 0.75f,
		-2.0f, 0.0f, 2.0f,	0.75f, 0.75f, 0.75f,

		2.0f, 0.0f, 1.0f,	0.75f, 0.75f, 0.75f,
		-2.0f, 0.0f, 1.0f,	0.75f, 0.75f, 0.75f,

		2.0f, 0.0f, 0.0f,	0.75f, 0.75f, 0.75f,
		-2.0f, 0.0f, 0.0f,	0.75f, 0.75f, 0.75f,

		2.0f, 0.0f, -1.0f,	0.75f, 0.75f, 0.75f,
		-2.0f, 0.0f, -1.0f,	0.75f, 0.75f, 0.75f,

		2.0f, 0.0f, -2.0f, 	0.75f, 0.75f, 0.75f,
		-2.0f, 0.0f, -2.0f,	0.75f, 0.75f, 0.75f,

		2.0f, 0.0f, 2.0f,	0.75f, 0.75f, 0.75f,
		2.0f, 0.0f, -2.0f, 	0.75f, 0.75f, 0.75f,

		1.0f, 0.0f, 2.0f, 	0.75f, 0.75f, 0.75f,
		1.0f, 0.0f, -2.0f, 	0.75f, 0.75f, 0.75f,

		0.0f, 0.0f, 2.0f, 	0.75f, 0.75f, 0.75f,
		0.0f, 0.0f, -2.0f, 	0.75f, 0.75f, 0.75f,

		-1.0f, 0.0f, 2.0f,	0.75f, 0.75f, 0.75f,
		-1.0f, 0.0f, -2.0f,	0.75f, 0.75f, 0.75f,

		-2.0f, 0.0f, 2.0f, 	0.75f, 0.75f, 0.75f,
		-2.0f, 0.0f, -2.0f,	0.75f, 0.75f, 0.75f
	};


	unsigned int floorVAO;
	glGenVertexArrays(1, &floorVAO);

	unsigned int floorVBO;
	glGenBuffers(1, &floorVBO);

	glBindVertexArray(floorVAO);
	glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floorVectorsData), floorVectorsData, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Init callbacks
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouseKey_callback);

	glViewport(0, 0, width, height);

	glEnable(GL_DEPTH_TEST);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		do_movement();

		// calculate delta time
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Clear screen
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 viewMat = camera.GetViewMatrix();

		// Draw
		// Draw basis line
		glm::mat4 modelMat;
		lineShader.Use();
		lineShader.setMat4("modelMat", modelMat);
		lineShader.setMat4("viewMat", viewMat);
		lineShader.setMat4("projMat", projMat);

		glBindVertexArray(basisVAO);
		glDrawArrays(GL_LINES, 0, 6);
		glBindVertexArray(0);

		modelMat = glm::scale(modelMat, glm::vec3(10.0f, 10.0f, 10.0f));
		lineShader.setMat4("modelMat", modelMat);
		glBindVertexArray(floorVAO);
		glDrawArrays(GL_LINES, 0, 20);
		glBindVertexArray(0);

		bvhAnimation.update(deltaTime);

		skelLineShader.Use();
		skelLineShader.setMat4("viewMat", viewMat);
		skelLineShader.setMat4("projMat", projMat);

		bvhAnimation.draw(&skelLineShader);

		// Swap Buffers
		glfwSwapBuffers(window);
	}

	glDeleteVertexArrays(1, &basisVAO);
	glDeleteVertexArrays(1, &floorVAO);
	glDeleteBuffers(1, &basisVBO);
	glDeleteBuffers(1, &floorVBO);

	glfwTerminate();

	return 0;
}