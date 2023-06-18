#pragma once
#include <vector>
#include "Triangle.h"
struct SingleHouse
{
	int count;//�������
	std::vector<float> vertexs;//��������
	std::vector<unsigned int> indexs;//��������
	std::vector<float> boundings;//��Χ���� xmin xmax ymin ymax zmin zmax
	QVector<Triangle> triangles;//�����ʷֵĽ��

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