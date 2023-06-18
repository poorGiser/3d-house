#include "FileProcess.h"
#include <fstream>
#include<iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <qdebug.h>
#include "Triangle.h"
#include "MyPoint.h"

std::vector<std::string> split(const std::string& str, const std::string& delim) { //分割字符串
	std::vector<std::string> res;
	if ("" == str) return res;
	//先将要切割的字符串从string类型转换为char*类型
	char* strs = new char[str.length() + 1];
	strcpy(strs, str.c_str());

	char* d = new char[delim.length() + 1];
	strcpy(d, delim.c_str());

	char* p = strtok(strs, d);
	while (p) {
		std::string s = p; //分割得到的字符串转换为string类型
		res.push_back(s); //存入结果数组
		p = strtok(NULL, d);
	}

	return res;
}

bool isCounterclockwise(const QVector<MyPoint> polygon)//判断顺时针或逆时针，true:逆，false：顺
{
	qreal area = 0;
	int n = polygon.size();

	for (int i = 0; i < n; ++i) {
		const MyPoint& p1 = polygon[i];
		const MyPoint& p2 = polygon[(i + 1) % n];
		area += (p2.x - p1.x) * (p2.y + p1.y);
	}

	return area < 0;
}

glm::vec3 computeNormal(std::vector<float> points,bool sOrN) {
	int pointSize = 3;
	glm::vec3 vec1(points[pointSize * 2] - points[0], points[pointSize * 2 + 1] - points[1], points[pointSize * 2 + 2] - points[2]);
	glm::vec3 vec2(points[pointSize] - points[0], points[pointSize + 1] - points[1], points[pointSize + 2] - points[2]);
	glm::vec3 normalVec = glm::cross(vec1, vec2);

	if (!sOrN) {
		return normalVec;
	}
	return -normalVec;
}

float* computeBox(std::vector<float> points) {//计算侧面包围盒
	float* box = new float[4];//xmin,xmax,ymin,ymax
	int pointSize = 3;

	for (int i = 0; i < 4; i++) {
		float x = points[pointSize * i];
		float y = points[pointSize * i + 1];
		if (i == 0) {
			box[0] = x;
			box[1] = x;
			box[2] = y;
			box[3] = y;
		}
		else {
			box[0] = x < box[0] ? x : box[0];
			box[1] = x > box[1] ? x : box[1];
			box[2] = y < box[2] ? y : box[2];
			box[3] = y > box[3] ? y : box[3];
		}
	}

	return box;
}

bool isConcavePolygon(const QVector<QPointF>& points)//判断多边形是否是凹多边形
{
	int size = points.size();
	bool sign = false;
	for (int i = 0; i < size; ++i) {
		QPointF p1 = points[i];
		QPointF p2 = points[(i + 1) % size];
		QPointF p3 = points[(i + 2) % size];
		qreal crossProduct = (p2.x() - p1.x()) * (p3.y() - p2.y()) -
			(p2.y() - p1.y()) * (p3.x() - p2.x());
		if (i == 0) {
			sign = crossProduct > 0;
		}
		else if (sign != (crossProduct > 0)) {
			return true;
		}
	}
	return false;
}

bool isPit(QVector<QPointF> triangle, bool sOrN) {//判断是否是凹点
	QPointF p1 = triangle[0];
	QPointF p2 = triangle[1];
	QPointF p3 = triangle[2];
	double crossProduct = (p2.x() - p1.x()) * (p3.y() - p2.y()) -
		(p2.y() - p1.y()) * (p3.x() - p2.x());
	if (sOrN == false) {
		return crossProduct > 0;
	}
	else {
		return crossProduct < 0;
	}
}

bool isContain(QVector<QPointF> triangle, MyPoint point, bool sOrN) {//判断三角形是否包含某个点
	for (int i = 0; i < 3; i++) {
		QPointF p1 = triangle[i];
		QPointF p2 = triangle[(i + 1) % 3];
		QPointF p12 = QPointF(p2.x()-p1.x(),p2.y()-p1.y());
		QPointF p1p = QPointF(point.x - p1.x(), point.y - p1.y());
		float dot = p12.x() * p1p.y() - p12.y() * p1p.x();
		if (sOrN == false) {
			if (dot > 0) {
				return false;
			}
		}
		else {
			if (dot < 0) {
				return false;
			}
		}
	}

	return true;
}

QVector<Triangle> triangulation(QVector<MyPoint> points,bool sOrN) {//耳切三角剖分算法
	QVector<Triangle> triangles;
	int currentIndex = 0;
	while(true){
		int len = points.size();
		Triangle triangle(points[currentIndex % len].index, points[(currentIndex + 1) % len].index, points[(currentIndex + 2) % len].index);
		if (points.size() == 3) {//只有三个点
			triangles.push_back(triangle);
			break;
		}

		//判断当前点的下一个点是不是凹点
		QVector<QPointF> tempTriangle;

		for (int j = 0; j < 3; j++) {
			QPointF point(points[(currentIndex + j) % len].x, points[(currentIndex + j) % len].y);
			tempTriangle.push_back(point);
		}

		bool pitResult = isPit(tempTriangle,sOrN);

		if (pitResult) {//第二个点是凹点
			//qDebug() << (currentIndex + 1) % len << endl;
			currentIndex = (currentIndex + 1) % len;
			continue;
		}

		//判断是否包含其他点
		bool isCon = false;
		for (int k = 0; k < points.size(); k ++) {
			if (points[k].index != points[currentIndex % len].index && points[k].index != points[(currentIndex + 1) % len].index && points[k].index != points[(currentIndex + 2) % len].index) {
				if (isContain(tempTriangle, points[k],sOrN)) {
					isCon = true;
					break;
				}
			}
		}

		if (isCon) {
			currentIndex = (currentIndex + 1) % len;
			continue;
		}

		//满足条件
		triangles.push_back(triangle);//加入三角形
		points.erase(points.begin() + (currentIndex + 1) % len);
	}

	return triangles;
}

//根据housebox计算纹理坐标
float* getTexCoord(float houseXmin, float houseXmax, float houseYmin, float houseYmax,float x,float y,bool isTop) {
	float* tex = new float[2];
	tex[0] = (x - houseXmin )/(houseXmax - houseXmin);
	if (isTop) {
		tex[1] = 1.0 - (y - houseYmin) / (houseYmax - houseYmin);
	}
	else {
		tex[1] = (y - houseYmin) / (houseYmax - houseYmin);
	}
	return tex;
}


FileProcess::FileProcess(QObject *parent)
	: QObject(parent)
{}

FileProcess::~FileProcess()
{}


std::string FileProcess::getContent(std::string path)
{
	std::ifstream infile;
	infile.open(path, std::ios::in);
	if (!infile.is_open())
	{
		std::cout << "读取文件失败" << std::endl;
		return "";
	}
	std::string content="";
	std::string buf;
	while (std::getline(infile, buf))
	{
		content.append(buf);
		content += "\n";
	}
	infile.close();
	return content;
}

bool FileProcess::getHouseData(std::string path, std::vector<float>& points, std::vector<unsigned int>& index, std::vector<float>& houseTopAndBottomVertexs, std::vector<unsigned int>& houseTopAndBottomIndexs, std::vector<SingleHouse>& houses, std::vector<float>& lineHouseVertexs, std::vector<unsigned int>& lineHouseIndexs)
{
	std::ifstream infile;
	infile.open(path, std::ios::in);
	if (!infile.is_open())
	{
		return false;
	}

	std::string buf;
	int count = 0;
	int house_num = 1;
	int pointSize = 3;//每个点的包含x,y,z
	int vertexStride = 5;

	//纹理坐标数组
	float textureCoords[4][2] = {0.0,0.0,0.0,1.0,1.0,0.0,1.0,1.0};
	float textureTopBottomCoords[4][2] = { 0.0,0.0,0.0,1.0,1.0,1.0,1.0,0.0 };
	//存储顶点坐标
	std::vector<float> rawPoint;

	//存储单house的包围盒
	float houseXmin;
	float houseXmax;
	float houseYmin;
	float houseYmax;
	float houseZmin;
	float houseZmax;

	while (std::getline(infile, buf))
	{
		//读取数据存入数组
		std::vector<std::string> splitResult = split(buf," ");

		if (splitResult.size() < 4) { //读取到每个建筑的首行(id)和末行(0)
			if (buf.find(std::to_string(house_num)) != std::string::npos) continue; //首行

			if (buf.find("0") != std::string::npos) {//末行
				for (int i = 0; i < pointSize * 2; i++) {//去除最后重复的两个点
					rawPoint.pop_back();
				}
				count -= 2;

				//计算vertexs数组
				//顶面和底面

				//获取底面点数据
				QVector<MyPoint> bottom;
				for (int i = 0; i < count / 2; i++)
				{
					MyPoint point(rawPoint[2 * i * pointSize], rawPoint[2 * i * pointSize + 2], rawPoint[2 * i * pointSize + 1], i);
					bottom.push_back(point);
				}

				//获取顶面点数据
				QVector<MyPoint> top;
				for (int i = 0; i < count / 2; i++)
				{
					MyPoint point(rawPoint[(2 * i + 1) * pointSize], rawPoint[(2 * i + 1) * pointSize + 2], rawPoint[(2 * i + 1) * pointSize + 1], i);
					top.push_back(point);
				}

				bool isc = isCounterclockwise(bottom);

				//1 侧面
				for (int i = 0; i < count; i+=2) {
					std::vector<float> tempPoints;
					for (int j = 0; j < 4; j++) {
						for (int k = 0; k < pointSize; k++) {
							tempPoints.push_back(rawPoint[(i + j) % count * pointSize + k]);
						}
					}

					glm::vec3 normal = computeNormal(tempPoints, isc);
					float* box = computeBox(tempPoints);

					for (int j = 0; j < 4; j++) {//一个侧面有四个点
						for (int k = 0; k < pointSize; k++) {
							points.push_back(rawPoint[(i + j) % count * pointSize + k]);
						}

						//顶点纹理坐标
						float* tex = getTexCoord(box[0], box[1], box[2], box[3], rawPoint[(i + j) % count * pointSize], rawPoint[(i + j) % count * pointSize + 1],false);
						points.push_back(tex[0]);
						points.push_back(tex[1]);

						//顶点法向量
						points.push_back(normal.x);
						points.push_back(normal.y);
						points.push_back(normal.z);
					}
				}

				//进行三角剖分
				QVector<Triangle> triangles = triangulation(bottom,isc);

				//顶点数据
				for (int i = 0; i < triangles.size(); i++) {
					Triangle triangle = triangles[i];

					//计算法向量
					std::vector<float> tempPoints;
					for (int j = 0; j < 3; j++) {
						tempPoints.push_back(top[triangle.getPointIndex(j)].x);
						tempPoints.push_back(top[triangle.getPointIndex(j)].z);
						tempPoints.push_back(top[triangle.getPointIndex(j)].y);
					}

					glm::vec3 normal = computeNormal(tempPoints, isc);
					normal = -normal;

					//顶面
					for (int j = 0; j < 3; j++) {//依次加入三个顶点
						points.push_back(top[triangle.getPointIndex(j)].x);
						points.push_back(top[triangle.getPointIndex(j)].z);
						points.push_back(top[triangle.getPointIndex(j)].y);
						
						float* tex = getTexCoord(houseXmin, houseXmax, houseYmin, houseYmax, top[triangle.getPointIndex(j)].x, top[triangle.getPointIndex(j)].y,true);

						points.push_back(tex[0]);
						points.push_back(tex[1]);

						points.push_back(normal.x);
						points.push_back(normal.y);
						points.push_back(normal.z);
					}

					normal = -normal;


					//底面
					for (int j = 0; j < 3; j++) {//依次加入三个顶点
						points.push_back(bottom[triangle.getPointIndex(j)].x);
						points.push_back(bottom[triangle.getPointIndex(j)].z);
						points.push_back(bottom[triangle.getPointIndex(j)].y);

						float* tex = getTexCoord(houseXmin, houseXmax, houseYmin, houseYmax, bottom[triangle.getPointIndex(j)].x, bottom[triangle.getPointIndex(j)].y,true);
						points.push_back(tex[0]);
						points.push_back(tex[1]);

						points.push_back(normal.x);
						points.push_back(normal.y);
						points.push_back(normal.z);
					}
				}

				unsigned int total = points.size() / 8;//（3 + 2 + 3）
				int sideNum = count / 2;//侧面的个数
				int topAndBottomNum = triangles.size() * 3 * 2;//顶面和底面点的个数
				//unsigned int start  = total - sideNum * 4 - count;
				unsigned int start = total - sideNum * 4 - topAndBottomNum;
				
				//计算index
				//1 侧面
				for (int i = 0; i < sideNum; i++) {
					for (int j = 0; j < 2; j++) {
						for (int k = 0; k < 3; k++) {
							index.push_back(start + 4 * i + j + k);
						}
					}
				}

				//2 底面和顶面
				for (int i = 0; i < bottom.size() * 3 * 2; i+=3) {
					index.push_back(start + sideNum * 4 + i);
					index.push_back(start + sideNum * 4 + i + 1);
					index.push_back(start + sideNum * 4 + i + 2);
				}

				//单体House
				houses.push_back(SingleHouse(count, rawPoint, houseXmin,houseXmax,houseZmin,houseZmax,houseYmin,houseYmax,triangles));

				/*线模式*/
				//顶点数据
				int linePointSize = 3;
				for (int i = 1; i <= count; i++) {
					lineHouseVertexs.push_back(rawPoint[linePointSize * (i - 1)]);
					lineHouseVertexs.push_back(rawPoint[linePointSize * (i - 1) + 1]);
					lineHouseVertexs.push_back(rawPoint[linePointSize * (i - 1) + 2]);
				}

				unsigned int total2 = lineHouseVertexs.size() / 3;
				int lineStart = total2 - count;
				//索引数据
				for (int i = 0; i < count; i+=2) {//侧边
					lineHouseIndexs.push_back(lineStart + i);
					lineHouseIndexs.push_back(lineStart + i + 1);
				}

				for (int i = 1; i < count; i += 2) {//顶边
					lineHouseIndexs.push_back(lineStart + i);
					lineHouseIndexs.push_back(lineStart + (i + 2) % count);
				}

				for (int i = 0; i < count; i += 2) {//底边
					lineHouseIndexs.push_back(lineStart + i);
					lineHouseIndexs.push_back(lineStart + (i + 2) % count);
				}

				count = 0;
				house_num += 1;
				rawPoint.clear();
				continue;
			}
		}

		//读取一行数据
		float x = std::stof(splitResult[0]);
		float y = std::stof(splitResult[1]);
		float zmax = std::stof(splitResult[2]);
		float zmin = std::stof(splitResult[3]);

		//更新boundary
		if (house_num == 1 && count == 0) {//初始化
			this->xmin = x;
			this->xmax = x;
			this->ymin = y;
			this->ymax = y;
			this->zmin = zmin;
			this->zmax = zmax;
		}
		else {//更新
			this->xmin = x < this->xmin ? x : this->xmin;
			this->xmax = x > this->xmax ? x : this->xmax;

			this->ymin = y < this->ymin ? y : this->ymin;
			this->ymax = y > this->ymax ? y : this->ymax;

			this->zmin = zmin < this->zmin ? zmin : this->zmin;
			this->zmax = zmax > this->zmax ? zmax : this->zmax;
		}

		//更新house包围盒
		if (count == 0) {//初始化
			houseXmin = x;
			houseXmax = x;
			houseYmax = y;
			houseYmin = y;
			houseZmin = zmin;
			houseZmax = zmax;
		}
		else {
			houseXmin = x < houseXmin ? x : houseXmin;
			houseXmax = x > houseXmax ? x : houseXmax;
			houseYmin = y < houseYmin ? y : houseYmin;
			houseYmax = y > houseYmax ? y : houseYmax;
			houseZmin = zmin < houseZmin ? zmin : houseZmin;
			houseZmax = zmax > houseZmax ? zmax : houseZmax;
		}

		rawPoint.push_back(x);
		rawPoint.push_back(zmin);
		rawPoint.push_back(y);
		count += 1;

		rawPoint.push_back(x);
		rawPoint.push_back(zmax);
		rawPoint.push_back(y);
		count += 1;
	}
	infile.close();
	return true;
}
