#include "drawing.h"

const float X_Offset = 10;
const float Y_OffSet = 10;

double scaleFactor;
double moveX;
double moveY;

void calculateDrawParameters()
{
	double tmp1 = (WINDOW_WIDTH - 20) / (maxX - minX);
	double tmp2 = (WINDOW_HEIGHT - 20) / (maxY - minY);

	scaleFactor = tmp1 < tmp2 ? tmp1 : tmp2;
	moveX = -minX;
	moveY = -minY;
}

void translateAndScale(Point &pt)
{
	pt.x += moveX;
	pt.y += moveY;

	pt.x *= scaleFactor;
	pt.y *= scaleFactor;
}

void drawTriangle(Point a, Point b, Point c, sf::RenderWindow* window)
{
	translateAndScale(a);
	translateAndScale(b);
	translateAndScale(c);

	float y_size = window->getSize().y;
	sf::VertexArray lines(sf::LinesStrip, 4);
	lines[0].position = sf::Vector2f(a.x - 1.5 + X_Offset, y_size - a.y - 1.5 - Y_OffSet);
	lines[1].position = sf::Vector2f(b.x - 1.5 + X_Offset, y_size - b.y - 1.5 - Y_OffSet);
	lines[2].position = sf::Vector2f(c.x - 1.5 + X_Offset, y_size - c.y - 1.5 - Y_OffSet);
	lines[3].position = sf::Vector2f(a.x - 1.5 + X_Offset, y_size - a.y - 1.5 - Y_OffSet);
	window->draw(lines);
}

void drawCenter(Point a, sf::RenderWindow* window)
{
	translateAndScale(a);
	sf::CircleShape circle(2);
	circle.setFillColor(sf::Color(0, 255, 0));
	circle.setPosition(sf::Vector2f(a.x - 1.5 + X_Offset, window->getSize().y - a.y - 1.5 - Y_OffSet));
	window->draw(circle);
}

void drawTriangle(Triangle &triangle, Point* points, sf::RenderWindow* window)
{
	float y_size = window->getSize().y;
	Point a = points[triangle.vertexIndexes[0]];
	Point b = points[triangle.vertexIndexes[1]];
	Point c = points[triangle.vertexIndexes[2]];

	translateAndScale(a);
	translateAndScale(b);
	translateAndScale(c);

	sf::VertexArray lines(sf::LinesStrip, 4);
	lines[0].position = sf::Vector2f(a.x + X_Offset, y_size - a.y - Y_OffSet);
	lines[1].position = sf::Vector2f(b.x + X_Offset, y_size - b.y - Y_OffSet);
	lines[2].position = sf::Vector2f(c.x + X_Offset, y_size - c.y - Y_OffSet);
	lines[3].position = sf::Vector2f(a.x + X_Offset, y_size - a.y - Y_OffSet);
	window->draw(lines);

	sf::CircleShape circle(2);
	circle.setFillColor(sf::Color(255, 0, 0));
	circle.setPosition(sf::Vector2f(a.x - 1.5 + X_Offset, y_size - a.y - 1.5 - Y_OffSet));
	window->draw(circle);
	
	circle.setPosition(sf::Vector2f(b.x - 1.5 + X_Offset, y_size - b.y - 1.5 - Y_OffSet));
	window->draw(circle);

	circle.setPosition(sf::Vector2f(c.x - 1.5 + X_Offset, y_size - c.y - 1.5 - Y_OffSet));
	window->draw(circle);
}