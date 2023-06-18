#pragma once
struct MyPoint {
	float x;
	float y;
	float z;
	int index;

	MyPoint(float x, float y, float z, int index):x(x),y(y),z(z),index(index) {}
	MyPoint(float x, float y, float z) :x(x), y(y), z(z),index(-1) {}

};