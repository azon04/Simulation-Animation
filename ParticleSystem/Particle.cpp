#include "Particle.h"

#include <cstdlib>

float slowSaturation = 0.7f;
float fastSaturation = 1.0f;
float slowValue = 0.7f;
float fastValue = 1.0f;
float slowHue = 0.5f;
float fastHue = 0.0f;
int HueDirection = 1;

glm::vec3 hsv2rgb(float h, float s, float v)
{
	glm::vec3 rgb;
	float h6 = 6 * h;
	float r, g, b;
	float coef;

	if (h6 < 1)
	{
		r = 0;
		g = 1 - h6;
		b = 1;
	}
	else if (h6 < 2)
	{
		r = h6 - 1;
		g = 0;
		b = 1;
	}
	else if (h6 < 3)
	{
		r = 1;
		g = 0;
		b = 3 - h6;
	}
	else if (h6 < 4)
	{
		r = 1;
		g = h6 - 3;
		b = 0;
	}
	else if (h6 < 5)
	{
		r = 5 - h6;
		g = 1;
		b = 0;
	}
	else
	{
		r = 0;
		g = 1;
		b = h6 - 5;
	}

	coef = v * s;
	rgb.r = v - coef * r;
	rgb.g = v - coef * g;
	rgb.b = v - coef * b;

	return rgb;
}

float getHue(float coef)
{
	float hue;
	float sh = slowHue;
	float fh = fastHue;

	if (sh < fh && HueDirection == 0)
	{
		sh += 1;
	}
	else if (sh > fh && HueDirection == 1)
	{
		fh += 1;
	}

	hue = (1 - coef) * sh + coef * fh;
	if (hue > 1.0f)
	{
		hue -= 1;
	}

	return hue;
}

float getHSVValue(float coef)
{
	return (1 - coef) * slowValue + coef * fastValue;
}

float getSaturation(float coef)
{
	return (1 - coef) * slowSaturation + coef * fastSaturation;
}

float getSpeedCoef(glm::vec2 speed)
{
	float coef;
	coef = log(speed.x * speed.x + speed.y * speed.y + 1) / 4.5f;
	if (coef > 1.0f)
	{
		coef = 1.0f;
	}
	return coef;
}

void initParticleSystem(ParticleSystem* particleSystem, int width, int height)
{
	for (unsigned int i = 0; i < PARTICLE_NUMBER; i++)
	{
		particleSystem->particle_p[i] = glm::vec3( (rand() % width ), (rand() % height), 0.0f);
	}

	for (unsigned int i = 0; i < PARTICLE_NUMBER; i++)
	{
		float coef = getSpeedCoef(particleSystem->particle_v[i]);
		particleSystem->particle_c[i] = hsv2rgb(getHue(coef), getSaturation(coef), getHSVValue(coef));
	}

	for (unsigned int i = 0; i < EFFECTOR_NUMBER; i++)
	{
		particleSystem->effectors[i] = glm::vec3(0.1f * i, 0.5 - 0.2f * i, 0.0f);
	}
}

void updateParticleSystem(ParticleSystem* particleSystem, float deltaTime)
{
	const float fAttraction = 15000.0f;
	const float fDragCoef = 0.98f;

	for (unsigned int i = 0; i < PARTICLE_NUMBER; i++)
	{
		glm::vec2& acceleration = particleSystem->particle_a[i];
		glm::vec2& velocity = particleSystem->particle_v[i];
		glm::vec2& pos = particleSystem->particle_p[i];
		glm::vec3& color = particleSystem->particle_c[i];
		acceleration.x = 0.0;
		acceleration.y = 0.0;

		for (unsigned int j = 0; j < particleSystem->numberOfEffector; j++)
		{
			glm::vec2 particleToEffector = (particleSystem->effectors[j] - pos);
			float diffSqNorm = particleToEffector.x * particleToEffector.x + particleToEffector.y * particleToEffector.y;
			acceleration += particleToEffector * (fAttraction / diffSqNorm);
		}

		velocity += acceleration * deltaTime;

		float coef = getSpeedCoef(velocity / 50.0f);
		color = hsv2rgb(getHue(coef), getSaturation(coef), getHSVValue(coef));

		pos += velocity * deltaTime + acceleration * deltaTime * deltaTime * 0.5f;
		velocity *= fDragCoef;
	}
}

void updateParticleSystemIndexes(ParticleSystem* particleSystem, float deltaTime, int start, int count)
{
	const float fAttraction = 15000.0f;
	const float fDragCoef = 0.98f;

	for (unsigned int i = start; i < start+count-1; i++)
	{
		glm::vec2& acceleration = particleSystem->particle_a[i];
		glm::vec2& velocity = particleSystem->particle_v[i];
		glm::vec2& pos = particleSystem->particle_p[i];
		glm::vec3& color = particleSystem->particle_c[i];
		acceleration.x = 0.0;
		acceleration.y = 0.0;

		for (unsigned int j = 0; j < particleSystem->numberOfEffector; j++)
		{
			glm::vec2 particleToEffector = (particleSystem->effectors[j] - pos);
			float diffSqNorm = particleToEffector.x * particleToEffector.x + particleToEffector.y * particleToEffector.y;
			acceleration += particleToEffector * (fAttraction / diffSqNorm);
		}

		velocity += acceleration * deltaTime;

		float coef = getSpeedCoef(velocity / 50.0f);
		color = hsv2rgb(getHue(coef), getSaturation(coef), getHSVValue(coef));

		pos += velocity * deltaTime + acceleration * deltaTime * deltaTime * 0.5f;
		velocity *= fDragCoef;
	}
}
