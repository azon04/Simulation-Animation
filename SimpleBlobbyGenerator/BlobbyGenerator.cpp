#include "BlobbyGenerator.h"

#define SQUARE(a) ((a)*(a))

#define TOINDEX(x, y) ((y)*m_resolutionX + (x))

BlobbyGenerator::BlobbyGenerator()
{
	setup();
}

void BlobbyGenerator::GenerateGridValues()
{
	for (int y = 0; y < m_resolutionY; y++)
	{
		for (int x = 0; x < m_resolutionX; x++)
		{
			m_grid[TOINDEX(x, y)] = 0.0f;
			for (int c = 0; c < m_circles.size(); c++)
			{
				Circle& circle = m_circles[c];
				float posX = x * m_gridGranuality.x;
				float posY = y * m_gridGranuality.y;
				m_grid[TOINDEX(x,y)] += SQUARE(circle.m_radius) / (SQUARE(posX - circle.m_position.x) + SQUARE(posY - circle.m_position.y));
			}
		}
	}
}
float interp(float from, float to, float rate)
{
	return from + (to - from) * rate;
}

void BlobbyGenerator::GenerateBlobbiesPoints(std::vector<glm::vec2>& buffer)
{
	for (int y = 0; y < m_resolutionY - 1; y++)
	{
		for (int x = 0; x < m_resolutionX - 1; x++)
		{
			glm::vec2 pos(x * m_gridGranuality.x, y * m_gridGranuality.y);

			int value = 0;
			value |= (m_grid[TOINDEX(x, y)] >= 1.0f ? 1 : 0) << 3;
			value |= (m_grid[TOINDEX(x+1, y)] >= 1.0f ? 1 : 0) << 2;
			value |= (m_grid[TOINDEX(x+1, y+1)] >= 1.0f ? 1 : 0) << 1;
			value |= (m_grid[TOINDEX(x, y+1)] >= 1.0f ? 1 : 0);

			std::vector<glm::vec2>& vertices = m_table[value];
			for (int i = 0; i < vertices.size(); i++)
			{
				//buffer.push_back(pos + vertices[i] * m_gridSize);
				glm::vec2 newPos = pos + vertices[i] * m_gridGranuality;
				if (vertices[i].x == 0.0f || vertices[i].x == 1.0f)
				{
					int iterpX = x + vertices[i].x;
					float a = (1 - m_grid[TOINDEX(iterpX, y)]);
					float b = (m_grid[TOINDEX(iterpX, y + 1)] - m_grid[TOINDEX(iterpX, y)]);
					newPos.y = interp(pos.y, pos.y + m_gridGranuality.y,
						a / b);
				}

				if (vertices[i].y == 0.0f || vertices[i].y == 1.0f)
				{
					int iterpY = y + vertices[i].y;
					float a = (1 - m_grid[TOINDEX(x, iterpY)]);
					float b = (m_grid[TOINDEX(x+1, iterpY)] - m_grid[TOINDEX(x, iterpY)]);
					newPos.x = interp(pos.x, pos.x + m_gridGranuality.x,
						a / b);
				}

				buffer.push_back(newPos);
			}
		}
	}
}

glm::vec2 reflect(glm::vec2 dir, glm::vec2 norm)
{
	return dir - 2 * glm::dot(dir, norm) * norm;
}

void BlobbyGenerator::Update(float _deltaTime)
{
	for (int i = 0; i < m_circles.size(); i++)
	{
		Circle& circle = m_circles[i];
		circle.m_position += circle.m_direction * _deltaTime;

		// check windows collision
		float directionLength = glm::length(circle.m_direction);
		if(directionLength == 0.0f) { continue; }
		glm::vec2 positionToTest = circle.m_position + ((circle.m_radius / directionLength) * circle.m_direction );
		if (positionToTest.x < 0.0f && glm::dot(circle.m_direction, glm::vec2(1.0f, 0.0f)) < 0.0f)
		{
			circle.m_direction = reflect(circle.m_direction, glm::vec2(1.0f, 0.0f));
		}
		else if (positionToTest.x > m_width && glm::dot(circle.m_direction, glm::vec2(-1.0f, 0.0f)) < 0.0f)
		{
			circle.m_direction = reflect(circle.m_direction, glm::vec2(-1.0f, 0.0f));
		}
		else if (positionToTest.y < 0.0f && glm::dot(circle.m_direction, glm::vec2(0.0f, 1.0f)) < 0.0f)
		{
			circle.m_direction = reflect(circle.m_direction, glm::vec2(0.0f, 1.0f));
		}
		else if (positionToTest.y > m_height && glm::dot(circle.m_direction, glm::vec2(0.0f, -1.0f)) < 0.0f)
		{
			circle.m_direction = reflect(circle.m_direction, glm::vec2(0.0f, -1.0f));
		}
	}
}

void BlobbyGenerator::SetGridGranuality(int _gridGranuality)
{
	m_gridGranuality = glm::vec2(_gridGranuality, _gridGranuality);

	m_resolutionX = m_width / m_gridGranuality.x + 1;
	m_resolutionY = m_height / m_gridGranuality.y + 1;

	m_grid.resize(m_resolutionY * m_resolutionX);
}

void BlobbyGenerator::Init(int width, int height, int gridGran)
{
	m_width = width;
	m_height = height;

	SetGridGranuality(gridGran);
}

void BlobbyGenerator::setup()
{
	// 0000 = 0
	{
		std::vector<glm::vec2> vertices;
		m_table.push_back(vertices);
	}

	// 0001 = 1
	{
		std::vector<glm::vec2> vertices;
		vertices.push_back(glm::vec2(0.0f, 0.5f));
		vertices.push_back(glm::vec2(0.5f, 1.0f));
		m_table.push_back(vertices);
	}

	// 0010 = 2
	{
		std::vector<glm::vec2> vertices;
		vertices.push_back(glm::vec2(1.0f, 0.5f));
		vertices.push_back(glm::vec2(0.5f, 1.0f));
		m_table.push_back(vertices);
	}

	// 0011 = 3
	{
		std::vector<glm::vec2> vertices;
		vertices.push_back(glm::vec2(1.0f, 0.5f));
		vertices.push_back(glm::vec2(0.0f, 0.5f));
		m_table.push_back(vertices);
	}

	// 0100 = 4
	{
		std::vector<glm::vec2> vertices;
		vertices.push_back(glm::vec2(1.0f, 0.5f));
		vertices.push_back(glm::vec2(0.5f, 0.0f));
		m_table.push_back(vertices);
	}

	// 0101 = 5
	{
		std::vector<glm::vec2> vertices;
		vertices.push_back(glm::vec2(0.0f, 0.5f));
		vertices.push_back(glm::vec2(0.5f, 0.0f));
		vertices.push_back(glm::vec2(1.0f, 0.5f));
		vertices.push_back(glm::vec2(0.5f, 1.0f));
		m_table.push_back(vertices);
	}

	// 0110 = 6
	{
		std::vector<glm::vec2> vertices;
		vertices.push_back(glm::vec2(0.5f, 0.0f));
		vertices.push_back(glm::vec2(0.5f, 1.0f));
		m_table.push_back(vertices);
	}

	// 0111 = 7
	{
		std::vector<glm::vec2> vertices;
		vertices.push_back(glm::vec2(0.0f, 0.5f));
		vertices.push_back(glm::vec2(0.5f, 0.0f));
		m_table.push_back(vertices);
	}

	// 1000 = 8
	{
		std::vector<glm::vec2> vertices;
		vertices.push_back(glm::vec2(0.0f, 0.5f));
		vertices.push_back(glm::vec2(0.5f, 0.0f));
		m_table.push_back(vertices);
	}

	// 1001 = 9
	{
		std::vector<glm::vec2> vertices;
		vertices.push_back(glm::vec2(0.5f, 0.0f));
		vertices.push_back(glm::vec2(0.5f, 1.0f));
		m_table.push_back(vertices);
	}

	// 1010 = 10
	{
		std::vector<glm::vec2> vertices;
		vertices.push_back(glm::vec2(0.0f, 0.5f));
		vertices.push_back(glm::vec2(0.5f, 0.0f));
		vertices.push_back(glm::vec2(1.0f, 0.5f));
		vertices.push_back(glm::vec2(0.5f, 1.0f));
		m_table.push_back(vertices);
	}

	// 1011 = 11
	{
		std::vector<glm::vec2> vertices;
		vertices.push_back(glm::vec2(1.0f, 0.5f));
		vertices.push_back(glm::vec2(0.5f, 0.0f));
		m_table.push_back(vertices);
	}

	// 1100 = 12
	{
		std::vector<glm::vec2> vertices;
		vertices.push_back(glm::vec2(0.0f, 0.5f));
		vertices.push_back(glm::vec2(1.0f, 0.5f));
		m_table.push_back(vertices);
	}

	// 1101 = 13
	{
		std::vector<glm::vec2> vertices;
		vertices.push_back(glm::vec2(1.0f, 0.5f));
		vertices.push_back(glm::vec2(0.5f, 1.0f));
		m_table.push_back(vertices);
	}

	// 1110 = 14
	{
		std::vector<glm::vec2> vertices;
		vertices.push_back(glm::vec2(0.0f, 0.5f));
		vertices.push_back(glm::vec2(0.5f, 1.0f));
		m_table.push_back(vertices);
	}

	// 1111 = 15
	{
		std::vector<glm::vec2> vertices;
		m_table.push_back(vertices);
	}
}
