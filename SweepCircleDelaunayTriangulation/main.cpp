/*

Algoritem: Delaunayjeva triangulacija s prebirnim krogom (sweep-circle Delaunay triangulation)
Avtor: Gal Meznariè

v1.2 | april 2017


opombe:
- izris ogromne kolièine toèk (veè 100k) je za SFML problematièno
- toèke triangulacije so oznaèene z rdeèo, središèe O ("prebirnega kroga") pa z zeleno.

poganjanje programa:
./program.exe <ime_datoteke> [0 ~ uporabi 1. inic. | 1 ~ uporabi 2. inic (default)] [0 ~ ne izriši | 1 ~ izriši (default)] [0 ~ brez polnjenja kotlin | 1 ~ s polnjenjem kotlin (default)]

*/

#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <cmath>
#include <chrono>
#include <sstream>
#include "drawing.h"
using namespace std;

Point* points; // array vseh toèk iz datoteke
Triangle* triangles; // vsi zgenerirani trikotniki v triangulaciji
Point center; // središèe prebirnega kroga == težišèe toèk
int currentTriangleCount = 0; // trenutno št. trikotnikov v triangulaciji 
int pointsCount = 0; // št. vseh toèk iz datoteke
multimap<double, int> advancingFront; // key: kot ; value: index toèke v polju toèk
sf::RenderWindow* window;
typedef chrono::high_resolution_clock Clock;
int initialisationMode = 0; // katero inicializacijo uporabljamo (0 == okolica (0, 0) ; 1 == center triangulacije)

double scalarProduct(int left, int center, int right) // "kot" med (left-center) in (right-center) + orientacija
{
	double left_x = points[left].x - points[center].x;
	double left_y = points[left].y - points[center].y;
	double right_x = points[right].x - points[center].x;
	double right_y = points[right].y - points[center].y;

	double orientation = left_x*right_y - left_y*right_x;

	if (orientation >= 0)
		return -1.0f;

	return left_x*right_x + left_y*right_y;
}

double crossProduct(int first, int second, const int third) // med (first, second) in (first, third) ... return: negativno --> desno orientira T; pozitivno --> levo orientiran T
{
	return (points[second].x - points[first].x) * (points[third].y - points[first].y) - (points[third].x - points[first].x) * (points[second].y - points[first].y);
}

bool sortComparator(Point &a, Point &b)
{
	if (a.r == b.r)
		return a.angle < b.angle;
	return a.r < b.r;
}

void readData(string fname)
{
	// indeksi toèk prvega trikotnika
	int min1 = 0;
	int min2 = 0;
	int min3 = 0;
	Point pt;
	int firstAddCount = 0;
	double current_distance;
	double distance1 = DBL_MAX;
	double distance2 = DBL_MAX;
	double distance3 = DBL_MAX;

	try
	{
		ifstream file(fname);
		if (!file.good())
		{
			cerr << "napaka pri branju datoteke" << endl;
			exit(1);
		}

		cout << "branje podatkov..." << endl;
		// preberi št. toèk v datoteki
		file >> pointsCount;

		points = new Point[pointsCount + 3]; // rezervirajmo pomnilnik za toèke ... 3, ki jih bomo rabili za 1. trikotnik, bomo dali na konec
		pointsCount = 0;

		// beri podatke (toèke) in izbiraj toèke za prvi trikotnik (3 najbližje neidentiène toèke (0, 0)
		string line;
		bool odd = false;
		char delim = ',';
		while (getline(file, line, delim))
		{
			stringstream ss(line);
			if (!odd)
			{
				ss >> pt.x;
				odd = true;
				delim = '\n';
				continue;
			}
			else
			{
				ss >> pt.y;
				odd = false;
				delim = ',';
			}

			points[pointsCount] = pt;

			if (pt.x > maxX)
				maxX = pt.x;
			if (pt.x < minX)
				minX = pt.x;
			if (pt.y > maxY)
				maxY = pt.y;
			if (pt.y < minY)
				minY = pt.y;

			if (initialisationMode == 0)
			{
				double current_distance = pt.x*pt.x + pt.y*pt.y; // kvadratna evklidska razdalja med toèko in koordinatnim izhodišèem

				if (pointsCount == 0)
				{
					distance1 = current_distance;
				}

				// tab-ception
				if (!(points[min3].x == pt.x && points[min3].y == pt.y))
				{
					if (current_distance < distance3)
					{
						if (!(points[min2].x == pt.x && points[min2].y == pt.y))
						{
							if (current_distance <= distance2)
							{
								if (!(points[min1].x == pt.x && points[min1].y == pt.y))
								{
									if (current_distance <= distance1 && !(points[min1].x == pt.x && points[min1].y == pt.y))
									{
										min3 = min2;
										distance3 = distance2;
										min2 = min1;
										distance2 = distance1;
										min1 = pointsCount;
										distance1 = current_distance;
									}
									else
									{
										min3 = min2;
										distance3 = distance2;
										min2 = pointsCount;
										distance2 = current_distance;
									}
								}
								else
								{
									points[pointsCount].validState = 0;
								}
							}
							else
							{
								min3 = pointsCount;
								distance3 = current_distance;
							}
						}
						else
						{
							points[pointsCount].validState = 0;
						}
					}
				}
				else
				{
					points[pointsCount].validState = 0;
				}
			}

			pointsCount++;
		}

		if (initialisationMode == 0 && (distance1 == DBL_MAX || distance2 == DBL_MAX || distance3 == DBL_MAX))
		{
			cerr << "premalo (neidenticnih) tock za triangulacijo." << endl;
			exit(1);
		}

		if (initialisationMode == 0)
		{
			center.x = (points[min1].x + points[min2].x + points[min3].x) / 3.0;
			center.y = (points[min1].y + points[min2].y + points[min3].y) / 3.0;

			points[pointsCount] = points[min1];
			points[pointsCount + 1] = points[min2];
			points[pointsCount + 2] = points[min3];

			points[pointsCount].r = hypotf(points[pointsCount].x - center.x, points[pointsCount].y - center.y); // evklidska razdalja med toèko in centrom kroga;
			points[pointsCount + 1].r = hypotf(points[pointsCount + 1].x - center.x, points[pointsCount + 1].y - center.y);
			points[pointsCount + 2].r = hypotf(points[pointsCount + 2].x - center.x, points[pointsCount + 2].y - center.y);

			points[min1].validState = false;
			points[min2].validState = false;
			points[min3].validState = false;

			sort(points + pointsCount, points + pointsCount + 3, sortComparator); // sortiraj zadnje tri, da bo ob inicializaciji pravi vrstni red na AF
		}
		else
		{
			center.x = (maxX + minX) / 2.0;
			center.y = (maxY + minY) / 2.0;
		}

		cout << "branje podatkov uspesno, izbran center: " << center.x << " " << center.y << endl;
	}
	catch (exception e)
	{
		cerr << "branje podatkov neuspesno!" << endl;
		exit(1);
	}

}

void carthesianToPolar()
{
	for (int i = 0; i != pointsCount + 3; i++)
	{
		points[i].r = hypotf(points[i].x - center.x, points[i].y - center.y); // evklidska razdalja med toèko in centrom kroga
		points[i].angle = atan2f(points[i].y - center.y, points[i].x - center.x); // kot
		if (points[i].angle < 0)
			points[i].angle += 2 * M_PI; // [-PI, PI] --> [0, 2PI]
	}
}

int initialize()
{
	int ret = 0;
	triangles[currentTriangleCount] = Triangle();
	if (initialisationMode == 0)
	{
		// dodaj prve 3 toèke v AF
		int j = 0;
		for (int i = pointsCount; i != pointsCount + 3; i++)
		{
			advancingFront.insert(pair<double, int>(points[i].angle, int(i)));
			points[i].triangleIndex = 0;
			triangles[0].vertexIndexes[j] = i;
			j++;
		}
	}
	else
	{
		int ptCount = 0;

		for (int i = 0; i != pointsCount; i++)
		{
			if (ptCount == 0 || points[i] != points[triangles[0].vertexIndexes[ptCount - 1]])
			{
				points[pointsCount + ptCount] = points[i];
				advancingFront.insert(pair<double, int>(points[pointsCount + ptCount].angle, int(pointsCount + ptCount)));
				points[pointsCount + ptCount].triangleIndex = 0;
				triangles[0].vertexIndexes[ptCount] = pointsCount + ptCount;

				ptCount++;
			}

			if (ptCount == 3)
			{
				ret = i + 1;
				break;
			}
		}

		// premakni center v težišèe trikotnika
		center.x = (points[triangles[0].vertexIndexes[0]].x + points[triangles[0].vertexIndexes[1]].x + points[triangles[0].vertexIndexes[2]].x) / 3.0;
		center.y = (points[triangles[0].vertexIndexes[0]].y + points[triangles[0].vertexIndexes[1]].y + points[triangles[0].vertexIndexes[2]].y) / 3.0;
	}

	currentTriangleCount++;

	return ret;
}

void legalize(int triangleIndex, int edgeIndex)
{
	// nimamo preko danega roba niè za legalizirati ?
	if (triangleIndex == -1 || triangles[triangleIndex].neighbourTriangleIndexes[edgeIndex] == -1)
		return;

	// https://en.wikipedia.org/wiki/Circumscribed_circle#Cartesian_coordinates_2
	// Circumcenter coordinates
	Point a = points[triangles[triangleIndex].vertexIndexes[0]];
	Point b = points[triangles[triangleIndex].vertexIndexes[1]];
	Point c = points[triangles[triangleIndex].vertexIndexes[2]];
	double D = 2 * (a.x*(b.y - c.y) + b.x*(c.y - a.y) + c.x*(a.y - b.y));

	double circle_x = (1.0f / D) * ((a.x*a.x + a.y*a.y)*(b.y - c.y) + (b.x*b.x + b.y*b.y)*(c.y - a.y) + (c.x*c.x + c.y*c.y)*(a.y - b.y));
	double circle_y = (1.0f / D) * ((a.x*a.x + a.y*a.y)*(c.x - b.x) + (b.x*b.x + b.y*b.y)*(a.x - c.x) + (c.x*c.x + c.y*c.y)*(b.x - a.x));
	double r_2 = (a.x - circle_x)*(a.x - circle_x) + (a.y - circle_y)*(a.y - circle_y);

	// poišèi vrh sosednjega trikotnika
	int currentVertex = -1;
	for (int i = 0; i != 3; i++)
	{
		currentVertex = triangles[triangles[triangleIndex].neighbourTriangleIndexes[edgeIndex]].vertexIndexes[i]; // dobi trenutno oglišèe v sosednjem trikotniku
																												  // je to oglišèe skupno obema (sosednjemu in trenutnemu) trikotniku ?
		if (currentVertex == triangles[triangleIndex].vertexIndexes[0] || currentVertex == triangles[triangleIndex].vertexIndexes[1] || currentVertex == triangles[triangleIndex].vertexIndexes[2])
			continue;
		else
			break;
	}

	Point v = points[currentVertex];
	// je vrh izven ali na oèrtani krožnici ? ... r_2 malo zmanjšamo, za numerièno stabilnost
	if ((v.x - circle_x) * (v.x - circle_x) + (v.y - circle_y) * (v.y - circle_y) >= (0.99999 * r_2))
	{
		// potem sta trikotnika legalna in zakljuèimo to vejo legalizacije
		return;
	}
	// legalizacija
	else
	{
		// indexi vertexov za novo stanje
		int newTop1; // nov vrh trikotnika newTriangle1
		int newTop2; // nov vrh trikotniak newTriangle2
		if (edgeIndex == 1)
		{
			newTop1 = triangles[triangleIndex].vertexIndexes[0];
			newTop2 = triangles[triangleIndex].vertexIndexes[2];
		}
		else if (edgeIndex == 2)
		{
			newTop1 = triangles[triangleIndex].vertexIndexes[1];
			newTop2 = triangles[triangleIndex].vertexIndexes[0];
		}
		else
		{
			newTop1 = triangles[triangleIndex].vertexIndexes[2];
			newTop2 = triangles[triangleIndex].vertexIndexes[1];
		}

		int newLeft = currentVertex; // levo oglišèe trikotnika newTriangle1 <--> desno oglišèe newTriangle2
		int newRight = triangles[triangleIndex].vertexIndexes[edgeIndex]; // desno oglišèe trikotnika newTriangle1 <--> levo oglišèe newTriangle2
		Triangle newTriangle1;
		Triangle newTriangle2;

		// popravi na kateri trikotnik kažejo toèke na AF ... èe ne kaže na katerega od teh, ki jih bomo swapali, jih pusti na miru, ker mora kazati na najbolj desni trikotnik v triangulaciji
		if (points[newTop1].triangleIndex == triangleIndex)
		{
			points[newTop1].triangleIndex = triangles[triangleIndex].neighbourTriangleIndexes[edgeIndex];
		}
		else if (points[newTop1].triangleIndex == triangles[triangleIndex].neighbourTriangleIndexes[edgeIndex])
		{
			points[newTop1].triangleIndex = triangleIndex;
		}
		if (points[newTop2].triangleIndex == triangles[triangleIndex].neighbourTriangleIndexes[edgeIndex])
		{
			points[newTop2].triangleIndex = triangleIndex;
		}
		else if (points[newTop2].triangleIndex == triangleIndex)
		{
			points[newTop2].triangleIndex = triangles[triangleIndex].neighbourTriangleIndexes[edgeIndex];
		}


		// osveži kazalce sosednjih trikotnikov, da bodo pravilni glede na novo stanje
		for (int i = 0; i != 3; i++)
		{
			// preko tega roba nima soseda ?
			if (triangles[triangleIndex].neighbourTriangleIndexes[i] == -1)
				continue;

			// preskoèi rob, ki ga bomo swapali
			if (newRight == triangles[triangleIndex].vertexIndexes[i])
				continue;

			// osveži trikotnik @triangleIndex
			{
				int importantPointer = -1; // kazalec drugega trikotnika, ki ga bomo spremenili 
				bool thisTriangle = false; // kaže na nov trikotnik ?
				for (int j = 0; j != 3; j++)
				{
					if (triangles[triangles[triangleIndex].neighbourTriangleIndexes[i]].neighbourTriangleIndexes[j] == triangleIndex)
						importantPointer = j;
					if (triangles[triangles[triangleIndex].neighbourTriangleIndexes[i]].vertexIndexes[j] == newTop1)
					{
						thisTriangle = true;
					}
				}
				if (thisTriangle)
				{
					triangles[triangles[triangleIndex].neighbourTriangleIndexes[i]].neighbourTriangleIndexes[importantPointer] = triangles[triangleIndex].neighbourTriangleIndexes[edgeIndex];
					newTriangle1.neighbourTriangleIndexes[0] = triangles[triangleIndex].neighbourTriangleIndexes[i];
				}
				else
				{
					triangles[triangles[triangleIndex].neighbourTriangleIndexes[i]].neighbourTriangleIndexes[importantPointer] = triangleIndex; // bv
					newTriangle2.neighbourTriangleIndexes[2] = triangles[triangleIndex].neighbourTriangleIndexes[i];
				}
			}
		}

		// osveži trikotnik @{sosed of triangleIndex}
		for (int i = 0; i != 3; i++)
		{
			// preko tega roba nima soseda ?
			if (triangles[triangles[triangleIndex].neighbourTriangleIndexes[edgeIndex]].neighbourTriangleIndexes[i] == -1)
				continue;

			// preskoèi rob, ki ga bomo swapali
			if (newLeft == triangles[triangles[triangleIndex].neighbourTriangleIndexes[edgeIndex]].vertexIndexes[i])
				continue;

			// osveži trikotnik
			{
				int importantPointer = -1; // kazalec drugega trikotnika, ki ga bomo spremenili 
				bool thisTriangle = false; // kaže na nov trikotnik ?
				for (int j = 0; j != 3; j++)
				{
					if (triangles[triangles[triangles[triangleIndex].neighbourTriangleIndexes[edgeIndex]].neighbourTriangleIndexes[i]].neighbourTriangleIndexes[j] == triangles[triangleIndex].neighbourTriangleIndexes[edgeIndex])
						importantPointer = j;
					if (triangles[triangles[triangles[triangleIndex].neighbourTriangleIndexes[edgeIndex]].neighbourTriangleIndexes[i]].vertexIndexes[j] == newTop2)
					{
						thisTriangle = true;
					}
				}
				if (thisTriangle)
				{
					triangles[triangles[triangles[triangleIndex].neighbourTriangleIndexes[edgeIndex]].neighbourTriangleIndexes[i]].neighbourTriangleIndexes[importantPointer] = triangleIndex;
					newTriangle2.neighbourTriangleIndexes[0] = triangles[triangles[triangleIndex].neighbourTriangleIndexes[edgeIndex]].neighbourTriangleIndexes[i];
				}
				else
				{
					triangles[triangles[triangles[triangleIndex].neighbourTriangleIndexes[edgeIndex]].neighbourTriangleIndexes[i]].neighbourTriangleIndexes[importantPointer] = triangles[triangleIndex].neighbourTriangleIndexes[edgeIndex];
					newTriangle1.neighbourTriangleIndexes[2] = triangles[triangles[triangleIndex].neighbourTriangleIndexes[edgeIndex]].neighbourTriangleIndexes[i];
				}
			}
		}

		// naredi nova trikotnika sosednja eden drugemu
		newTriangle1.neighbourTriangleIndexes[1] = triangleIndex;
		newTriangle2.neighbourTriangleIndexes[1] = triangles[triangleIndex].neighbourTriangleIndexes[edgeIndex];

		// vnesi vertexe v nov trikotnik
		newTriangle1.vertexIndexes[0] = newLeft;
		newTriangle1.vertexIndexes[1] = newTop1;
		newTriangle1.vertexIndexes[2] = newRight;
		newTriangle2.vertexIndexes[0] = newRight;
		newTriangle2.vertexIndexes[1] = newTop2;
		newTriangle2.vertexIndexes[2] = newLeft;

		// kopiraj nova trikotnika v množico trikotnikov na pravi mesti (nadomesti stara)
		triangles[triangles[triangleIndex].neighbourTriangleIndexes[edgeIndex]] = newTriangle1;
		triangles[triangleIndex] = newTriangle2;

		// rekurzivno preverjanje legalnosti preko 2 drugih relevantnih robov
		legalize(triangleIndex, 0);
		legalize(triangles[triangleIndex].neighbourTriangleIndexes[edgeIndex], 2);
	}
}

void moveLeftOnAF(multimap<double, int>::iterator &left, multimap<double, int>::iterator &leftleft) // premik levo po AF
{
	left = leftleft;
	leftleft--;

	if (leftleft == advancingFront.end() || leftleft->first != left->first)
	{
		// pomikaj se levo, do prvega el. z drugim keyem 
		leftleft = next(left);

		if (leftleft == advancingFront.end())
			leftleft = advancingFront.begin(); // nastavi levega od levega na zaèetek krožne fronte

		multimap<double, int>::iterator tmp;
		while (true)
		{
			tmp = next(leftleft);

			if (leftleft->first != left->first && (tmp == advancingFront.end() || tmp->first != leftleft->first))
			{
				break;
			}
			else
			{
				leftleft = tmp;
				if (leftleft == advancingFront.end())
					leftleft = advancingFront.begin();
			}
		}

		if (leftleft == advancingFront.end())
			leftleft = advancingFront.begin(); // nastavi levega od levega na zaèetek krožne fronte
	}
}

multimap<double, int>::iterator expandLeft(multimap<double, int>::iterator top, multimap<double, int>::iterator left) // input param: iterator na oglišèe levo od najnovejšega dodanega
{
	multimap<double, int>::iterator leftleft = prev(left);

	if (leftleft == advancingFront.end() || leftleft->first != left->first)
	{
		// pomikaj se levo, do prvega el. z drugim keyem 
		leftleft = next(left);

		if (leftleft == advancingFront.end())
			leftleft = advancingFront.begin(); // nastavi levega od levega na zaèetek krožne fronte

		multimap<double, int>::iterator tmp;
		while (true)
		{
			tmp = next(leftleft);

			if (leftleft->first != left->first && (tmp == advancingFront.end() || tmp->first != leftleft->first))
			{
				break;
			}
			else
			{
				leftleft = tmp;
				if (leftleft == advancingFront.end())
					leftleft = advancingFront.begin();
			}
		}

		if (leftleft == advancingFront.end())
			leftleft = advancingFront.begin(); // nastavi levega od levega na zaèetek krožne fronte
	}

	int prevTriangleIndex = points[left->second].triangleIndex; // kazalec na prejšnji trikotnik

	double scalarProd;
	while (true)
	{
		scalarProd = scalarProduct(leftleft->second, left->second, top->second);

		// kot veèji od 90° ?
		if (scalarProd < 0)
		{
			return left;
		}
		// kot manjši ali enak 90° --> dodali bomo trikotnik
		else
		{
			// dodaj nov trikotnik
			triangles[currentTriangleCount].vertexIndexes[0] = leftleft->second;
			triangles[currentTriangleCount].vertexIndexes[1] = top->second;
			triangles[currentTriangleCount].vertexIndexes[2] = left->second;

			int bottomTriangle = points[leftleft->second].triangleIndex; // sosed nasproti vrha (dobimo preko najbolj levega oglišèa)

																		 // posodobi sosede
			triangles[currentTriangleCount].neighbourTriangleIndexes[1] = bottomTriangle;
			triangles[currentTriangleCount].neighbourTriangleIndexes[0] = prevTriangleIndex;
			triangles[prevTriangleIndex].addNeighbourTriangleIndex(top->second, left->second, currentTriangleCount, points);
			triangles[bottomTriangle].addNeighbourTriangleIndex(leftleft->second, left->second, currentTriangleCount, points);

			// posodobimo kazalec na AF
			points[leftleft->second].triangleIndex = currentTriangleCount;

			// odstranimo "left" iz AF
			advancingFront.erase(left);

			// legalizacija
			legalize(currentTriangleCount, 0);
			legalize(currentTriangleCount, 1);

			prevTriangleIndex = points[leftleft->second].triangleIndex;

			// pripravimo za naslednjo iteracijo
			moveLeftOnAF(left, leftleft);

			currentTriangleCount++;
		}
	}
}

void removeBasinLeft(multimap<double, int>::iterator top, multimap<double, int>::iterator left) // input param: vrh in iterator na oglišèe levo od najnovejšega dodanega
{
	multimap<double, int>::iterator leftleft = prev(left);
	multimap<double, int>::iterator nextLeftleft;
	multimap<double, int>::iterator nextLeft;

	if (leftleft == advancingFront.end() || leftleft->first != left->first)
	{
		// pomikaj se levo, do prvega el. z drugim keyem 
		leftleft = next(left);

		if (leftleft == advancingFront.end())
			leftleft = advancingFront.begin(); // nastavi levega od levega na zaèetek krožne fronte

		multimap<double, int>::iterator tmp;
		while (true)
		{
			tmp = next(leftleft);

			if (leftleft->first != left->first && (tmp == advancingFront.end() || tmp->first != leftleft->first))
			{
				break;
			}
			else
			{
				leftleft = tmp;
				if (leftleft == advancingFront.end())
					break;
			}
		}

		if (leftleft == advancingFront.end())
			leftleft = advancingFront.begin(); // nastavi levega od levega na zaèetek krožne fronte
	}

	// basin detection
	double r2 = points[leftleft->second].r;
	double deltaR = points[top->second].r - r2;
	double deltaAngle = fmod(leftleft->first - top->first, 360.0);
	double result = deltaR / (r2 * deltaAngle);
	// not a basin ... (hevristièno doloèeno!) ?
	if (result <= 2.0f)
		return;

	// prestavimo se na zaèetek "kotline"
	top = leftleft;
	moveLeftOnAF(left, leftleft);
	moveLeftOnAF(left, leftleft);
	nextLeft = left;
	nextLeftleft = leftleft;
	moveLeftOnAF(nextLeft, nextLeftleft);

	int prevTriangleIndex = points[left->second].triangleIndex; // kazalec na prejšnji trikotnik

	double vectorProd = 0;
	bool loop = true;
	int loopCount = 0;
	while (loop)
	{
		vectorProd = crossProduct(left->second, leftleft->second, nextLeftleft->second);

		// levo orientiran ?
		if (vectorProd > 0)
		{
			loop = false; // našli smo lokalni maksimum in bomo zato v tej iteraciji potencialno zgenerirali zadnji trikotnik
		}
		// preverjanje robnega primera "lažno" zaznane kotline ?
		if (loopCount == 0)
		{
			// je leva toèka bolj oddaljena od središèa kot naš "vrh" in zato kotlina tu ni možna ?
			if (points[left->second].r > points[top->second].r)
				return;
		}

		// preverjanje pravilnosti potencialnega novega trikotnika
		vectorProd = crossProduct(top->second, left->second, leftleft->second);
		// je potencialni trikotnik levo orientiran ?
		if (vectorProd >= 0)
			return;

		// dodaj nov trikotnik
		triangles[currentTriangleCount].vertexIndexes[0] = leftleft->second;
		triangles[currentTriangleCount].vertexIndexes[1] = top->second;
		triangles[currentTriangleCount].vertexIndexes[2] = left->second;

		int bottomTriangle = points[leftleft->second].triangleIndex; // sosed nasproti vrha (dobimo preko najbolj levega oglišèa)

																	 // posodobi sosede
		triangles[currentTriangleCount].neighbourTriangleIndexes[1] = bottomTriangle;
		triangles[currentTriangleCount].neighbourTriangleIndexes[0] = prevTriangleIndex;
		triangles[prevTriangleIndex].addNeighbourTriangleIndex(top->second, left->second, currentTriangleCount, points);
		triangles[bottomTriangle].addNeighbourTriangleIndex(leftleft->second, left->second, currentTriangleCount, points);

		// posodobimo kazalec na AF
		points[leftleft->second].triangleIndex = currentTriangleCount;

		// odstranimo "left" iz AF
		advancingFront.erase(left);

		// legalizacija
		legalize(currentTriangleCount, 0);
		legalize(currentTriangleCount, 1);

		// pripravimo za naslednjo iteracijo
		prevTriangleIndex = points[leftleft->second].triangleIndex;

		left = nextLeft;
		leftleft = nextLeftleft;

		moveLeftOnAF(nextLeft, nextLeftleft);

		currentTriangleCount++;
		loopCount++;
	}
}

void moveRightOnAF(multimap<double, int>::iterator &right, multimap<double, int>::iterator &rightright, int &skipped) // premikaj se desno po AF
{
	// pomikaj se desno, do prvega el. z drugim keyem 
	right = rightright;

	// smo prej preskoèili nekaj toèk, ker smo se premaknili na najbližjo središèu od tistih, ki so kolinearne ?
	if (skipped > 0)
	{
		rightright = next(right);
		skipped--;
	}
	else
	{
		rightright = next(right); // premakni se, kot da imamo robni primer (2 kolinearni na AF)

								  // nismo na robnem primeru ?
		if (rightright == advancingFront.end() || rightright->first != right->first)
		{
			rightright = prev(right); // premakni, se kot da smo na obièajnem primeru

									  // smo padli prek ?
			if (rightright == advancingFront.end())
				rightright = prev(advancingFront.end()); // nastavi desnega od desnega na zaèetek krožne fronte

			if (rightright != advancingFront.end())
			{
				// pomikaj se k najbližji toèki središèu, ki je kolinearna z right-right
				multimap<double, int>::iterator tmp = rightright;
				bool secondCase = true;
				while (true)
				{
					tmp = prev(rightright);
					// kolinearno ?
					if (tmp == advancingFront.end() || tmp->first != rightright->first)
					{
						break;
					}
					else
					{
						skipped++;
						rightright = tmp;
						secondCase = false;
					}
				}

				// premikaj se, kot da sta right in right-right kolinearna
				while (secondCase)
				{
					tmp = next(rightright);
					// kolinearno ?
					if (tmp == advancingFront.end() || tmp->first != rightright->first)
					{
						break;
					}
					else
					{
						rightright = tmp;
					}
				}
			}
		}
	}

	// smo padli prek ?
	if (rightright == advancingFront.end())
		rightright = prev(advancingFront.end()); // nastavi desnega od desnega na konec krožne fronte
}

multimap<double, int>::iterator expandRight(multimap<double, int>::iterator top, multimap<double, int>::iterator right) // input param: iterator na oglišèe desno od najnovejšega dodanega
{
	int skipped = 0; // preskoèene "zaporedne" toèke na AF zaradi kolinearnosti

					 // pomikaj se desno, do prvega el. z drugim keyem 
	multimap<double, int>::iterator rightright = next(right); // premakni se, kot da imamo robni primer (2 kolinearni na AF)

															  // nismo na robnem primeru ?
	if (rightright == advancingFront.end() || rightright->first != right->first)
	{
		rightright = prev(right);

		// smo padli prek ?
		if (rightright == advancingFront.end())
			rightright = prev(advancingFront.end()); // nastavi desnega od desnega na zaèetek krožne fronte

		if (rightright != advancingFront.end())
		{
			multimap<double, int>::iterator tmp = rightright;
			while (true)
			{
				tmp = prev(rightright); // premikaj se po kolinearnem delu AF
										// kolinearno ?
				if (tmp == advancingFront.end() || tmp->first != rightright->first)
				{
					break;
				}
				else
				{
					rightright = tmp;
					skipped++;
				}
			}
		}
	}
	// smo padli prek ?
	if (rightright == advancingFront.end())
		rightright = prev(advancingFront.end()); // nastavi desnega od desnega na zaèetek krožne fronte

	int prevTriangleIndex = points[top->second].triangleIndex; // kazalec na prejšnji trikotnik

	double scalarProd = 0;
	while (true)
	{
		scalarProd = scalarProduct(top->second, right->second, rightright->second);

		// kot veèji od 90° ?
		if (scalarProd < 0)
		{
			return right;
		}
		// kot manjši ali enak 90° --> dodali bomo trikotnik
		else
		{
			// dodaj nov trikotnik
			triangles[currentTriangleCount].vertexIndexes[0] = right->second;
			triangles[currentTriangleCount].vertexIndexes[1] = top->second;
			triangles[currentTriangleCount].vertexIndexes[2] = rightright->second;

			int bottomTriangle = points[right->second].triangleIndex; // sosed nasproti vrha (dobimo preko najbolj levega oglišèa)

																	  // posodobi sosede
			triangles[currentTriangleCount].neighbourTriangleIndexes[1] = bottomTriangle;
			triangles[currentTriangleCount].neighbourTriangleIndexes[2] = prevTriangleIndex;
			triangles[prevTriangleIndex].addNeighbourTriangleIndex(top->second, right->second, currentTriangleCount, points);
			triangles[bottomTriangle].addNeighbourTriangleIndex(right->second, rightright->second, currentTriangleCount, points);

			// posodobimo kazalec na AF
			points[top->second].triangleIndex = currentTriangleCount;

			// odstranimo "left" iz AF
			advancingFront.erase(right);

			// legalizacija
			legalize(currentTriangleCount, 2);
			legalize(currentTriangleCount, 1);

			// pripravimo za naslednjo iteracijo
			prevTriangleIndex = points[top->second].triangleIndex;

			moveRightOnAF(right, rightright, skipped);

			currentTriangleCount++;
		}
	}
}

void removeBasinRight(multimap<double, int>::iterator top, multimap<double, int>::iterator right) // input param: iterator na oglišèe desno od najnovejšega dodanega
{
	int skipped = 0; // preskoèene "zaporedne" toèke na AF zaradi kolinearnosti

					 // pomikaj se desno, do prvega el. z drugim keyem 
	multimap<double, int>::iterator rightright = next(right); // premakni se, kot da imamo robni primer (2 kolinearni na AF)
	multimap<double, int>::iterator nextRightright;
	multimap<double, int>::iterator nextRight;

	// nismo na robnem primeru ?
	if (rightright == advancingFront.end() || rightright->first != right->first)
	{
		rightright = prev(right);

		if (rightright != advancingFront.end())
		{
			multimap<double, int>::iterator tmp = rightright;
			while (true)
			{
				tmp = prev(rightright); // premikaj se po kolinearnem delu AF
										// kolinearno ?
				if (tmp == advancingFront.end() || tmp->first != rightright->first)
				{
					break;
				}
				else
				{
					rightright = tmp;
					skipped++;
				}
			}
		}
	}

	// smo padli prek ?
	if (rightright == advancingFront.end())
		rightright = prev(advancingFront.end()); // nastavi desnega od desnega na zaèetek krožne fronte

												 // basin detection
	double r2 = points[rightright->second].r;
	double deltaR = points[top->second].r - r2;
	double deltaAngle = fmod(top->first - rightright->first, 360.0);
	double result = deltaR / (r2 * deltaAngle);
	// not a basin (hevristièno doloèeno!) ?
	if (result <= 2.0f)
		return;

	// prestavimo se na zaèetek "kotline"
	top = rightright;
	moveRightOnAF(right, rightright, skipped);
	moveRightOnAF(right, rightright, skipped);
	nextRight = right;
	nextRightright = rightright;
	moveRightOnAF(nextRight, nextRightright, skipped);

	int prevTriangleIndex = points[top->second].triangleIndex; // kazalec na prejšnji trikotnik

	double vectorProd = 0;
	bool loop = true;
	int loopCount = 0;
	while (loop)
	{
		vectorProd = crossProduct(right->second, rightright->second, nextRightright->second);

		// desno orientiran ?
		if (vectorProd < 0)
		{
			loop = false; // našli smo lokalni maksimum in bomo zato v tej iteraciji potencialno zgenerirali zadnji trikotnik
		}
		// preverjanje robnega primera "lažno" zaznane kotline ?
		if (loopCount == 0)
		{
			// je desna toèka bolj oddaljena od središèa kot naš "vrh" in zato kotlina tu ni možna ?
			if (points[right->second].r > points[top->second].r)
				return;
		}

		// preverjanje pravilnosti potencialnega novega trikotnika
		vectorProd = crossProduct(top->second, right->second, rightright->second);
		// je potencialni trikotnik desno orientiran ?
		if (vectorProd <= 0)
			return;

		// dodaj nov trikotnik
		triangles[currentTriangleCount].vertexIndexes[0] = right->second;
		triangles[currentTriangleCount].vertexIndexes[1] = top->second;
		triangles[currentTriangleCount].vertexIndexes[2] = rightright->second;

		int bottomTriangle = points[right->second].triangleIndex; // sosed nasproti vrha (dobimo preko najbolj levega oglišèa novega trikotnika)

																  // posodobi sosede
		triangles[currentTriangleCount].neighbourTriangleIndexes[1] = bottomTriangle;
		triangles[currentTriangleCount].neighbourTriangleIndexes[2] = prevTriangleIndex;
		triangles[prevTriangleIndex].addNeighbourTriangleIndex(top->second, right->second, currentTriangleCount, points);
		triangles[bottomTriangle].addNeighbourTriangleIndex(right->second, rightright->second, currentTriangleCount, points);

		// posodobimo kazalec na AF
		points[top->second].triangleIndex = currentTriangleCount;

		// odstranimo "left" iz AF
		advancingFront.erase(right);

		// legalizacija
		legalize(currentTriangleCount, 2);
		legalize(currentTriangleCount, 1);

		// pripravimo za naslednjo iteracijo
		prevTriangleIndex = points[top->second].triangleIndex;

		right = nextRight;
		rightright = nextRightright;

		moveRightOnAF(nextRight, nextRightright, skipped);

		currentTriangleCount++;
		loopCount++;
	}
}

void finalize() // premikanje v urini smeri, iskanje levo (pozitivno) orientiranih trikotnikov
{
	multimap<double, int>::iterator top = prev(advancingFront.end());
	multimap<double, int>::iterator prevTop = top;
	bool firstTop = true;

	int skipped = 0; // preskoèene "zaporedne" toèke na AF zaradi kolinearnosti

					 // pomikaj se desno, do prvega el. z drugim keyem 
	multimap<double, int>::iterator right = prev(top); // premakni se (hopefully) desno

													   // pomakni se na prvega dejansko desnega
	multimap<double, int>::iterator tmp = right;
	while (true)
	{
		tmp = prev(right); // premikaj se po kolinearnem delu AF
						   // kolinearno ?
		if (tmp->first != right->first)
		{
			break;
		}
		else
		{
			right = tmp;
		}
	}

	// dobi desnega od desnega
	tmp = top;
	multimap<double, int>::iterator rightright = right;
	moveRightOnAF(tmp, rightright, skipped);
	if (rightright->second == top->second)
		rightright = prev(right);

	double vectorProd = 0;
	int prevTriangleIndex = -1;
	// polni krog
	while (prevTop->first - top->first >= 0)
	{
		vectorProd = crossProduct(top->second, right->second, rightright->second);
		// pozitivna orientacija ?
		if (vectorProd > 0) // dodali bomo trikotnik
		{
			prevTriangleIndex = points[top->second].triangleIndex; // kazalec na prejšnji trikotnik

																   // dodaj nov trikotnik
			triangles[currentTriangleCount].vertexIndexes[0] = right->second;
			triangles[currentTriangleCount].vertexIndexes[1] = top->second;
			triangles[currentTriangleCount].vertexIndexes[2] = rightright->second;

			int bottomTriangle = points[right->second].triangleIndex; // sosed nasproti vrha (dobimo preko najbolj levega oglišèa novega trikotnika)

																	  // posodobi sosede
			triangles[currentTriangleCount].neighbourTriangleIndexes[1] = bottomTriangle;
			triangles[currentTriangleCount].neighbourTriangleIndexes[2] = prevTriangleIndex;
			triangles[prevTriangleIndex].addNeighbourTriangleIndex(top->second, right->second, currentTriangleCount, points);
			triangles[bottomTriangle].addNeighbourTriangleIndex(right->second, rightright->second, currentTriangleCount, points);

			// posodobimo kazalec na AF
			points[top->second].triangleIndex = currentTriangleCount;

			// odstranimo "left" iz AF
			advancingFront.erase(right);

			// legalizacija
			legalize(currentTriangleCount, 2);
			legalize(currentTriangleCount, 1);

			right = top;

			top = next(top); // pomakni top za nazaj (v levo)
			if (top == advancingFront.end())
				top = prev(advancingFront.end());
			prevTop = top;

			currentTriangleCount++;
		}
		// premaknili se bomo za 1 mesto naprej
		else
		{
			prevTop = top;
			top = right;
			moveRightOnAF(right, rightright, skipped);
			if (rightright->second == right->second || rightright->second == top->second)
			{
				rightright = prev(right);
				if (rightright == advancingFront.end())
					return;
			}

			firstTop = false;
		}
	}
}

int main(int argc, char** argv)
{
	int draw = 1; // default ... rišemo rezultat
	int removeBasins = 1; // default ... polnimo kotline
	string fname;
	if (argc >= 2)
		fname = string(argv[1]);
	else
	{
		cerr << "potrebno je podati datotetko s koordinatami tock (" + string(argv[0]) + " <ime_datoteke> [0|1] [0|1] [0|1] )\n ... prvi [0|1] : 0 ~ 1. inic (ref. pt.) (default) | 1 ~ 2. inic (pts. center)\n ... drugi [0|1] : 0 ~ ne izrisi | 1 ~ izrisi (default)\n ... tretji [0|1] : 0 ~ brez polnjenja kotlin | 1 ~ s polnjenjem kotlin (default)" << endl;
		return 1;
	}
	if (argc >= 3)
		initialisationMode = atoi(argv[2]);
	if (argc >= 4)
		draw = atoi(argv[3]);
	if (argc >= 5)
		removeBasins = atoi(argv[4]);

	readData(fname);

	auto start = Clock::now(); // branja podatkov ne štopamo
	carthesianToPolar();

	sort(points, points + pointsCount, sortComparator);

	triangles = new Triangle[2 * pointsCount - 5]; // radodarno rezervirajmo pomnilnik za trikotnike ... št. trikotnikov: 2 * n - 2 - k ... k je najmanj 3

												   // inicializacija
	int startIndex = initialize();

	auto initEnd = Clock::now();

	// prebirni krog ... triangulacija
	int lastAddedIndex = pointsCount + 2;
	for (int i = startIndex; i != pointsCount; i++)
	{
		// toèka iz prvotnega trikotnika, ali pa podvojena toèka ?
		if (points[i].validState == false || points[i].x == points[lastAddedIndex].x && points[i].y == points[lastAddedIndex].y)
		{
			points[i].validState = false;
			continue;
		}
		else
		{
			lastAddedIndex = i;
			if (initialisationMode == 1)
			{
				// popravi polarne koordinate
				points[i].r = hypotf(points[i].x - center.x, points[i].y - center.y); // evklidska razdalja med toèko in centrom kroga
				points[i].angle = atan2f(points[i].y - center.y, points[i].x - center.x); // kot
				if (points[i].angle < 0)
					points[i].angle += 2 * M_PI; // [-PI, PI] --> [0, 2PI]
			}
		}

		multimap<double, int>::iterator left = advancingFront.lower_bound(points[i].angle); // dobimo "levi" vertex na AF
		multimap<double, int>::iterator original = left;

		// smo "zgrešili" fronto ?
		if (left == advancingFront.end())
			left = advancingFront.begin();

		// pomakni se na zadnji (najbolj levi) enak key v multimap-u 
		multimap<double, int>::iterator tmp = left;
		while (true)
		{
			tmp = next(left);
			if (tmp == advancingFront.end() || tmp->first != left->first)
			{
				break;
			}
			else
			{
				left = tmp;
			}
		}
		if (left == advancingFront.end())
			left = advancingFront.begin();

		multimap<double, int>::iterator right = prev(original); // dobimo "desni" vertex na AF
																// smo "zgrešili" fronto ?
		if (right == advancingFront.end())
			right = prev(advancingFront.end());

		// premaknemo se na najbolj notranjega (glede na O), ki ima enak key
		while (true)
		{
			multimap<double, int>::iterator tmp = right;
			tmp = prev(right);
			if (tmp == advancingFront.end() || tmp->first != right->first)
				break;
			else
				right = tmp;
		}
		// smo "zgrešili" fronto ?
		if (right == advancingFront.end())
			right = prev(advancingFront.end());


		// dodaj nov trikotnik
		triangles[currentTriangleCount].neighbourTriangleIndexes[1] = points[left->second].triangleIndex; // sosed je nasproti vrha
		triangles[currentTriangleCount].vertexIndexes[0] = left->second;
		triangles[currentTriangleCount].vertexIndexes[1] = i;
		triangles[currentTriangleCount].vertexIndexes[2] = right->second;

		// dodaj nov vnos v AF
		multimap<double, int>::iterator top;
		if (original == advancingFront.end() || points[i].angle != original->first)
		{
			top = advancingFront.emplace_hint(original, pair<double, int>(points[i].angle, int(i)));
		}
		else
		{
			top = advancingFront.emplace_hint(next(left), pair<double, int>(points[i].angle, int(i)));
		}
		points[i].triangleIndex = currentTriangleCount;

		// posodobi "spodnji" trikotnik, da vemo, da ima soseda
		triangles[points[left->second].triangleIndex].addNeighbourTriangleIndex(left->second, right->second, currentTriangleCount, points);

		// popravi, da levi vertex na AF kaže na nov
		points[left->second].triangleIndex = currentTriangleCount;

		// legaliziraj nov trikotnik
		legalize(currentTriangleCount, 1);

		currentTriangleCount++;

		// propagacija v levo
		left = expandLeft(top, left);

		// propagacija v desno
		right = expandRight(top, right);

		// polnjenje kotlin v desno
		if (currentTriangleCount > 3 && removeBasins) // cca 4 trikotniki (7 toèk) so minimum za nastanek in uspešno polnjenje kotanje
			removeBasinRight(top, right);

		// polnjenje kotlin v levo
		if (currentTriangleCount > 3 && removeBasins) // cca 4 trikotniki (7 toèk) so minimum za nastanek in uspešno polnjenje kotanje
			removeBasinLeft(top, left);
	} // end triangulacija loop

	finalize(); // zakljuèi triangulacijo (dopolni do KL).

	auto end = Clock::now();

	cout << "trajanje inicializacije: " << chrono::duration_cast<chrono::milliseconds>(initEnd - start).count() << " ms" << endl;
	cout << "trajanje prebiranja: " << chrono::duration_cast<chrono::milliseconds>(end - initEnd).count() << " ms" << endl;
	cout << "skupaj: " << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl;

	// izris
	if (draw)
	{
		calculateDrawParameters();

		sf::ContextSettings settings;
		settings.antialiasingLevel = 8;
		window = new sf::RenderWindow(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "SFML window", sf::Style::Default, settings);
		window->setTitle("Izris triangulacije");

		cout << "risanje..." << endl;
		for (int i = 0; i != currentTriangleCount; i++)
		{
			drawTriangle(triangles[i], points, window);
		}

		drawCenter(center, window);

		window->display(); // rišemo samo enkrat
		cout << "risanje koncano!" << endl;

		// window management birokracija ... pri ogromnih kolièinah toèk (veè 100k) vèasih risanje ne uspe
		sf::Event event;
		while (window->isOpen())
		{
			sf::Event event;
			while (window->pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
				{
					window->close();
				}
			}
		}
	}

	delete[] points;
	delete[] triangles;
	return 0;
}