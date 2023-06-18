#pragma once
#include <vector>
#include "Triangle.h"
struct SingleHouse
{
	int count;//顶点个数
	std::vector<float> vertexs;//顶点数据
	std::vector<unsigned int> indexs;//索引数据
	std::vector<float> boundings;//包围盒子 xmin xmax ymin ymax zmin zmax
	QVector<Triangle> triangles;//三角剖分的结果

	SingleHouse(int count, std::vector<float> vertexs) {
		this->count = count;
		this->vertexs = vertexs;
		//this->indexs = std::vector<unsigned int>();
	}

	SingleHouse(int count, std::vector<float> vertexs,float xmin,float xmax,float ymin, float ymax,float zmin,float zmax,QVector<Triangle> triangles) {
		this->count = count;
		this->vertexs = vertexs;
		this->boundings.push_back(xmin);
		this->boundings.push_back(xmax);
		this->boundings.push_back(ymin);
		this->boundings.push_back(ymax);
		this->boundings.push_back(zmin);
		this->boundings.push_back(zmax);

		this->triangles = triangles;
	}
};