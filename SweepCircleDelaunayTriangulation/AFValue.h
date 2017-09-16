#pragma once
struct AFValue
{
public:
	int vertexIndex;

	AFValue() { }
	AFValue(int vertexIndex)
	{
		this->vertexIndex = vertexIndex;
	}
};