#pragma once

#include <vector>
#include "glm/glm.hpp"

class Circle
{
public:
	Circle(glm::vec2 _position, float _radius)
		: m_position(_position), m_radius(_radius), m_direction(0.0f, 0.0f)
	{}

	Circle(glm::vec2 _position, float _radius, glm::vec2 _direction)
		: m_position(_position), m_radius(_radius), m_direction(_direction)
	{}

	glm::vec2 m_position;
	glm::vec2 m_direction;
	float m_radius;
};

class BlobbyGenerator
{
public:
	BlobbyGenerator();

	void GenerateGridValues();
	void GenerateBlobbiesPoints(std::vector<glm::vec2>& buffer);
	void Update(float _deltaTime);
	void SetGridGranuality(int _gridGran);
	void Init(int width, int height, int gridGran);

protected:
	void setup();

public:
	std::vector<Circle> m_circles;

protected:
	int m_width;
	int m_height;
	int m_resolutionX;
	int m_resolutionY;
	std::vector<float> m_grid;
	std::vector<std::vector<glm::vec2>> m_table;
	glm::vec2 m_gridGranuality;
};