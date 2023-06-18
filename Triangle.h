#pragma once
struct Triangle {//三角形的顶点索引
	int index1;
	int index2;
	int index3;

	Triangle(int index1,int index2,int index3):index1(index1), index2(index2) , index3(index3) {}

	int getPointIndex(int i) {
		switch (i)
		{
		case 0:
			return index1;
		case 1:
			return index2;
		case 2:
			return index3;
		default:
			break;
		}
	}
};