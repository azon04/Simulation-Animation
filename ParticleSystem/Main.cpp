// CSCI 520 Animation and Simulation
// Homework 2
// Ahmad Fauzan Umar

// GLEW
#include <GL/glew.h>

// GLFW
#include "GLFW/glfw3.h"

#include "Shader.h"
#include "Particle.h"

#define MULTI_THREADING 1

#if MULTI_THREADING

#include <thread>
#include <chrono>

#define NUMBER_OF_THREAD 8

std::thread* threads[NUMBER_OF_THREAD];
bool programEnd = false;

#endif

GLfloat lastX = 400, lastY = 300;

// Time Tracker
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

int width, height;
int currentEffector = 0;
double mouseRelativeX, mouseRelativeY;

ParticleSystem particleSystem;

// TODO for NVIDIA Optimus :  This enable the program to use NVIDIA instead of integrated Intel graphics
#if WIN32 || WIN64
extern "C" {
#include <windows.h>
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}
#endif

#if MULTI_THREADING
void ThreadJob(int start)
{
	while (!programEnd)
	{
		updateParticleSystemIndexes(&particleSystem, 1/90.0f, start, PARTICLE_NUMBER / NUMBER_OF_THREAD);

		std::this_thread::sleep_for(std::chrono::milliseconds((long)(1000.0f / 90.0f)));
	}
}
#endif

void mouseKey_callback(GLFWwindow* window, int key, int action, int mode)
{
	if (key == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (action == GLFW_RELEASE)
		{
			particleSystem.effectors[currentEffector].x = mouseRelativeX;
			particleSystem.effectors[currentEffector].y = mouseRelativeY;

			currentEffector = (currentEffector + 1) % EFFECTOR_NUMBER;
			particleSystem.numberOfEffector++;
			if (particleSystem.numberOfEffector > EFFECTOR_NUMBER)
			{
				particleSystem.numberOfEffector = EFFECTOR_NUMBER;
			}
		}
	}
	else if (key == GLFW_MOUSE_BUTTON_RIGHT)
	{
		if (action == GLFW_RELEASE)
		{
			particleSystem.effectors[0].x = mouseRelativeX;
			particleSystem.effectors[0].y = mouseRelativeY;

			currentEffector = 1;
			particleSystem.numberOfEffector = 1;
		}
	}

}

void mouse_callback(GLFWwindow* windows, double xpos, double ypos)
{
	mouseRelativeX = xpos;
	mouseRelativeY = height - ypos;
}

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// Create Window
	GLFWwindow* window = glfwCreateWindow(1280, 720, "Particle System Homework", nullptr, nullptr);
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

	// Setup shader
	Shader shader("ParticleShader.vs", "ParticleShader.frag");

	// init particles
	initParticleSystem(&particleSystem, width, height);

	// Setup Buffer
	GLuint PosBuffer, ColorBuffer;
	glGenBuffers(1, &PosBuffer);
	glGenBuffers(1, &ColorBuffer);

	GLuint ParticleVAO;
	glGenVertexArrays(1, &ParticleVAO);

	glBindVertexArray(ParticleVAO);
	glBindBuffer(GL_ARRAY_BUFFER, PosBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(particleSystem.particle_p), particleSystem.particle_p, GL_STATIC_DRAW);

	// Position Attributes
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, ColorBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(particleSystem.particle_c), particleSystem.particle_c, GL_STATIC_DRAW);

	// Color Attributes
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	// Init callbacks
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouseKey_callback);

	// Setup OpenGL settings
	glEnable(GL_PROGRAM_POINT_SIZE);

	glViewport(0, 0, width, height);
	
	// Init Threads
#if MULTI_THREADING
	for (int i = 0; i < NUMBER_OF_THREAD; i++)
	{
		std::thread* thread = new std::thread(ThreadJob, i * (PARTICLE_NUMBER / NUMBER_OF_THREAD));
		threads[i] = thread;
	}
#endif

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		// calculate delta time
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Clear Screen
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		// Draw
		glBindVertexArray(ParticleVAO);
		shader.Use();
		shader.setFloat("width", width);
		shader.setFloat("height", height);
		glDrawArrays(GL_POINTS, 0, PARTICLE_NUMBER);
		glBindVertexArray(0);

		// Update Particle System
#if !MULTI_THREADING
		updateParticleSystem(&particleSystem, deltaTime);
#endif

		// Update buffer
		glBindBuffer(GL_ARRAY_BUFFER, PosBuffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(particleSystem.particle_p), particleSystem.particle_p);

		glBindBuffer(GL_ARRAY_BUFFER, ColorBuffer);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(particleSystem.particle_c), particleSystem.particle_c);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Swap Buffers
		glfwSwapBuffers(window);
	}

#if MULTI_THREADING
	programEnd = true;
	for (int i = 0; i < NUMBER_OF_THREAD; i++)
	{
		threads[i]->join();
		delete threads[i];
	}
#endif

	glDeleteBuffers(1, &PosBuffer);
	glDeleteBuffers(1, &ColorBuffer);
	glDeleteVertexArrays(1, &ParticleVAO);

	glfwTerminate();

	return 0;
}