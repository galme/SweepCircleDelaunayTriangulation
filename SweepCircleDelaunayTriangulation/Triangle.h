#pragma once
#include "Point.h"

struct Triangle
{
	int vertexIndexes[3]; // 0 - "levi" ; 1 - vrh ; 2 - "desni"
	int neighbourTriangleIndexes[3]; // 0 - nasproten ogli��a 0 ; 1 - nasproten ogli��a 1 ; 2 - nasproten ogli��a 2

	Triangle()
	{
		// init na "NULL"
		neighbourTriangleIndexes[0] = -1;
		neighbourTriangleIndexes[1] = -1;
		neighbourTriangleIndexes[2] = -1;
	}

	void addNeighbourTriangleIndex(int a, int b, int newNeighbour, Point* points) // param: indexa to�k, med katera ho�emo dodati sosednji trikotnik
	{ 
		if (vertexIndexes[0] == a && vertexIndexes[1] == b || vertexIndexes[0] == b && vertexIndexes[1] == a)
		{
			neighbourTriangleIndexes[2] = newNeighbour;
		}
		else if (vertexIndexes[1] == a && vertexIndexes[2] == b || vertexIndexes[1] == b && vertexIndexes[2] == a)
		{
			neighbourTriangleIndexes[0] = newNeighbour;
		}	
		else// if (vertexIndexes[0] == a && vertexIndexes[2] == b || vertexIndexes[0] == b && vertexIndexes[2] == a)
		{
			neighbourTriangleIndexes[1] = newNeighbour;
		}
		//else
		//	throw std::exception("WTF");
	}
};