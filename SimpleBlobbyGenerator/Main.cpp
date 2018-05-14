
// GLEW
#include <GL/glew.h>

// GLFW
#include "GLFW/glfw3.h"

#include <vector>
#include "Shader.h"
#include "MathHelper.h"
#include "BlobbyGenerator.h"

#define THREADING 0

#if THREADING

#include <thread>
#include <mutex>
#include <condition_variable>

std::thread* updateThread;
std::mutex updateLock;
std::condition_variable updateThreadVariable;

bool bProgramEnd = false;
#endif

int width, height;

// Time tracker
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

bool g_drawCircle = false;
bool g_play = true;

BlobbyGenerator generator;

std::vector<glm::vec2> buffer[2]; // double bufferring
int g_bufferToDraw = 0;

void key_callback(GLFWwindow* window, int key, int scanecode, int action, int mode)
{
	if (action == GLFW_RELEASE)
	{
		if (key == GLFW_KEY_C)
		{
			g_drawCircle = !g_drawCircle;
		}
		else
		{
			if (key == GLFW_KEY_P)
			{
				g_play = !g_play;
			}
		}
	}
}

void mouse_callback(GLFWwindow* window, double xPos, double yPos)
{
}

#if THREADING
void updateThreadJob()
{
	std::unique_lock<std::mutex> lck(updateLock);
	float _deltaTime = 0.0f;
	float _lastTime = 0.0f;

	while (!bProgramEnd)
	{
		// Calculate delta time
		GLfloat currentFrame = glfwGetTime();
		_deltaTime = currentFrame - _lastTime;
		_lastTime = currentFrame;

		if (g_play)
		{
			int bufferToUse = (g_bufferToDraw + 1) % 2;
			generator.Update(_deltaTime);
			generator.GenerateGridValues();
			buffer[bufferToUse].clear();
			generator.GenerateBlobbiesPoints(buffer[bufferToUse]);
			g_bufferToDraw = bufferToUse;
		}

		//updateThreadVariable.wait(lck);
		
	}
}
#endif

void readInput(BlobbyGenerator& blobby, const char* fileName)
{
	FILE* file = fopen(fileName, "r");

	if (file == nullptr)
	{
		return;
	}

	int gridGranuality = 0;
	fscanf(file, "%d", &gridGranuality);

	blobby.SetGridGranuality(gridGranuality);

	int circleCount = 0;
	fscanf(file, "%d", &circleCount);

	for (unsigned int i = 0; i < circleCount; i++)
	{
		float px, py, r, vx, vy;

		fscanf(file, "%f %f %f %f %f\n", &px, &py, &r, &vx, &vy);
		blobby.m_circles.push_back(Circle(glm::vec2(px, py), r, glm::vec2(vx, vy)));
	}

	fclose(file);
}

int main(int argc, char** argv)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);


	// Create Window
	GLFWwindow* window = glfwCreateWindow(1280, 720, "Blobby Generator", nullptr, nullptr);
	if (window == nullptr)
	{
		glfwTerminate();
		std::cout << "Failed to create GLFW window" << std::endl;
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Init glew
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

	// Init Blobbies Generator
	generator.Init(width, height, 0);
	if (argc > 1)
	{
		readInput(generator, argv[ 1 ]);
	}

	// Init Shaders
	Shader simpleLineShader("SimpleLineShader.vs", "SimpleColorShader.frag");

	// Init VAO and VBO

	std::vector<float> circleData;
	const float circleStep = 2 * PI / 32 ;
	float step = 0.0f;
	for (int i = 0; i < 32; i++)
	{
		circleData.push_back(cos(step));
		circleData.push_back(sin(step));
		step += circleStep;
	}

	// Circle VBO
	unsigned int circleVBO;
	glGenBuffers(1, &circleVBO);

	unsigned int circleVAO;
	glGenVertexArrays(1, &circleVAO);

	glBindVertexArray(circleVAO);
	glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * circleData.size(), &circleData[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	// Blobbies VBO
	generator.GenerateGridValues();
	generator.GenerateBlobbiesPoints(buffer[g_bufferToDraw]);

	unsigned int blobbiesVBO;
	glGenBuffers(1, &blobbiesVBO);

	unsigned int blobbiesVAO;
	glGenVertexArrays(1, &blobbiesVAO);

	glBindVertexArray(blobbiesVAO);
	glBindBuffer(GL_ARRAY_BUFFER, blobbiesVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * buffer[g_bufferToDraw].size(), &buffer[g_bufferToDraw][0].x, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glEnableVertexAttribArray(0);

	// Init callbacks
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);

	glViewport(0, 0, width, height);

#if THREADING
	updateThread = new std::thread(updateThreadJob);
#endif

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		
		// Calculate delta time
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

#if !THREADING
		if (g_play)
		{
			generator.Update(deltaTime);
			generator.GenerateGridValues();
			buffer[g_bufferToDraw].clear();
			generator.GenerateBlobbiesPoints(buffer[g_bufferToDraw]);
		}
#endif

		// Clear Screen
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Draw
		simpleLineShader.Use();
		simpleLineShader.setFloat("width", width);
		simpleLineShader.setFloat("height", height);

		glBindBuffer(GL_ARRAY_BUFFER, blobbiesVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * buffer[g_bufferToDraw].size(), &buffer[g_bufferToDraw][0].x, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		{
			glm::mat3 model;
			simpleLineShader.setMat3("modelMatrix", model);
			simpleLineShader.setVec3("color", glm::vec3(0.0f, 1.0f, 0.0f));
			glBindVertexArray(blobbiesVAO);
			glDrawArrays(GL_LINES, 0, buffer[g_bufferToDraw].size());
			glBindVertexArray(0);
		}

		if (g_drawCircle)
		{
			simpleLineShader.setVec3("color", glm::vec3(1.0f, 0.0f, 0.0f));
			glBindVertexArray(circleVAO);
			for (int i = 0; i < generator.m_circles.size(); i++)
			{
				glm::mat3 model;
				model = scale(model, glm::vec2(generator.m_circles[i].m_radius));
				model = translate(model, generator.m_circles[i].m_position);
				simpleLineShader.setMat3("modelMatrix", model);

				glDrawArrays(GL_LINE_LOOP, 0, circleData.size() / 2);
			}
			glBindVertexArray(0);
		}


		// Swap Buffer
		glfwSwapBuffers(window);
	}

	glDeleteVertexArrays(1, &circleVAO);
	glDeleteVertexArrays(1, &blobbiesVAO);
	glDeleteBuffers(1, &circleVBO);
	glDeleteBuffers(1, &blobbiesVBO);
	
	glfwTerminate();

#if THREADING
	updateThreadVariable.notify_all();
	bProgramEnd = true;
	updateThread->join();

	delete updateThread;
#endif

	return 0;
}