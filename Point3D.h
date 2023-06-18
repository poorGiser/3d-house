#pragma once
#include<qmath.h>
#include <QVector3D>
struct Point3D
{
	float x;
	float y;
	float z;

	Point3D(float x = 0.0,float y = 0.0,float z = 0.0):x(x),y(y),z(z) {

	}

	static float getDis(Point3D p1, Point3D p2) {
		QVector3D point1(p1.x, p1.y, p1.z);
		QVector3D point2(p2.x,p2.y,p2.z);

		return point1.distanceToPoint(point2);
	}
};