#pragma once
struct Point
{
	double x; // x koord v evklidskem KS
	double y; // y koord v ekvklidskem KS
	double r; // razdalja v polarnem KS
	double angle; // kot v polarnem KS
	int triangleIndex = -1; // kazalec na trikotnik (pomembno za toèke na AF)
	bool validState = true; // veljavna za triangulacijo ali ne (podvojena, ...)

	Point() { }
	Point(const float x, const float y)
	{
		this->x = x;
		this->y = y;
	}

	bool operator!=(Point &b)
	{
		return !(x == b.x && y == b.y);
	}
};