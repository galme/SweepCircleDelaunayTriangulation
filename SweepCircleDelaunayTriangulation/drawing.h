#pragma once
#include <SFML\Graphics.hpp>
#include "Triangle.h"
#include "globals.h"

void drawTriangle(Point a, Point b, Point c, sf::RenderWindow* window);
void drawTriangle(Triangle &triangle, Point* points, sf::RenderWindow* window);
void drawCenter(Point a, sf::RenderWindow* window);
void calculateDrawParameters();