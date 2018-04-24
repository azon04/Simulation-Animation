// CSCI 520 Animation and Simulation
// Homework 2
// Ahmad Fauzan Umar

#pragma once

#define PARTICLE_NUMBER 100000
#define EFFECTOR_NUMBER 5

#include "glm/glm.hpp"

struct ParticleSystem
{
	glm::vec2 particle_p[PARTICLE_NUMBER]; // Position
	glm::vec3 particle_c[PARTICLE_NUMBER]; // Color in HSV
	glm::vec2 particle_a[PARTICLE_NUMBER]; // Acceleration
	glm::vec2 particle_v[PARTICLE_NUMBER]; // Velocity

	glm::vec2 effectors[EFFECTOR_NUMBER]; // Number of Effector

	int numberOfEffector = 0;
};

void initParticleSystem(ParticleSystem* particleSystem, int width, int height);
void updateParticleSystem(ParticleSystem* particleSystem, float deltaTime); 
void updateParticleSystemIndexes(ParticleSystem* particleSystem, float deltaTime, int start, int count);
