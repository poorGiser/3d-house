#include "FileProcess.h"
#include <fstream>
#include<iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <qdebug.h>
#include "Triangle.h"
#include "MyPoint.h"

std::vector<std::string> split(const std::string& str, const std::string& delim) { //�ָ��ַ���
	std::vector<std::string> res;
	if ("" == str) return res;
	//�Ƚ�Ҫ�и���ַ�����string����ת��Ϊchar*����
	char* strs = new char[str.length() + 1];
	strcpy(strs, str.c_str());

	char* d = new char[delim.length() + 1];
	strcpy(d, delim.c_str());

	char* p = strtok(strs, d);
	while (p) {
		std::string s = p; //�ָ�õ����ַ���ת��Ϊstring����
		res.push_back(s); //����������
		p = strtok(NULL, d);
	}

	return res;
}

bool isCounterclockwise(const QVector<MyPoint> polygon)//�ж�˳ʱ�����ʱ�룬true:�棬false��˳
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

float* computeBox(std::vector<float> points) {//��������Χ��
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

bool isConcavePolygon(const QVector<QPointF>& points)//�ж϶�����Ƿ��ǰ������
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

bool isPit(QVector<QPointF> triangle, bool sOrN) {//�ж��Ƿ��ǰ���
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

bool isContain(QVector<QPointF> triangle, MyPoint point, bool sOrN) {//�ж��������Ƿ����ĳ����
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

QVector<Triangle> triangulation(QVector<MyPoint> points,bool sOrN) {//���������ʷ��㷨
	QVector<Triangle> triangles;
	int currentIndex = 0;
	while(true){
		int len = points.size();
		Triangle triangle(points[currentIndex % len].index, points[(currentIndex + 1) % len].index, points[(currentIndex + 2) % len].index);
		if (points.size() == 3) {//ֻ��������
			triangles.push_back(triangle);
			break;
		}

		//�жϵ�ǰ�����һ�����ǲ��ǰ���
		QVector<QPointF> tempTriangle;

		for (int j = 0; j < 3; j++) {
			QPointF point(points[(currentIndex + j) % len].x, points[(currentIndex + j) % len].y);
			tempTriangle.push_back(point);
		}

		bool pitResult = isPit(tempTriangle,sOrN);

		if (pitResult) {//�ڶ������ǰ���
			//qDebug() << (currentIndex + 1) % len << endl;
			currentIndex = (currentIndex + 1) % len;
			continue;
		}

		//�ж��Ƿ����������
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

		//��������
		triangles.push_back(triangle);//����������
		points.erase(points.begin() + (currentIndex + 1) % len);
	}

	return triangles;
}

//����housebox������������
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
		std::cout << "��ȡ�ļ�ʧ��" << std::endl;
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
	int pointSize = 3;//ÿ����İ���x,y,z
	int vertexStride = 5;

	//������������
	float textureCoords[4][2] = {0.0,0.0,0.0,1.0,1.0,0.0,1.0,1.0};
	float textureTopBottomCoords[4][2] = { 0.0,0.0,0.0,1.0,1.0,1.0,1.0,0.0 };
	//�洢��������
	std::vector<float> rawPoint;

	//�洢��house�İ�Χ��
	float houseXmin;
	float houseXmax;
	float houseYmin;
	float houseYmax;
	float houseZmin;
	float houseZmax;

	while (std::getline(infile, buf))
	{
		//��ȡ���ݴ�������
		std::vector<std::string> splitResult = split(buf," ");

		if (splitResult.size() < 4) { //��ȡ��ÿ������������(id)��ĩ��(0)
			if (buf.find(std::to_string(house_num)) != std::string::npos) continue; //����

			if (buf.find("0") != std::string::npos) {//ĩ��
				for (int i = 0; i < pointSize * 2; i++) {//ȥ������ظ���������
					rawPoint.pop_back();
				}
				count -= 2;

				//����vertexs����
				//����͵���

				//��ȡ���������
				QVector<MyPoint> bottom;
				for (int i = 0; i < count / 2; i++)
				{
					MyPoint point(rawPoint[2 * i * pointSize], rawPoint[2 * i * pointSize + 2], rawPoint[2 * i * pointSize + 1], i);
					bottom.push_back(point);
				}

				//��ȡ���������
				QVector<MyPoint> top;
				for (int i = 0; i < count / 2; i++)
				{
					MyPoint point(rawPoint[(2 * i + 1) * pointSize], rawPoint[(2 * i + 1) * pointSize + 2], rawPoint[(2 * i + 1) * pointSize + 1], i);
					top.push_back(point);
				}

				bool isc = isCounterclockwise(bottom);

				//1 ����
				for (int i = 0; i < count; i+=2) {
					std::vector<float> tempPoints;
					for (int j = 0; j < 4; j++) {
						for (int k = 0; k < pointSize; k++) {
							tempPoints.push_back(rawPoint[(i + j) % count * pointSize + k]);
						}
					}

					glm::vec3 normal = computeNormal(tempPoints, isc);
					float* box = computeBox(tempPoints);

					for (int j = 0; j < 4; j++) {//һ���������ĸ���
						for (int k = 0; k < pointSize; k++) {
							points.push_back(rawPoint[(i + j) % count * pointSize + k]);
						}

						//������������
						float* tex = getTexCoord(box[0], box[1], box[2], box[3], rawPoint[(i + j) % count * pointSize], rawPoint[(i + j) % count * pointSize + 1],false);
						points.push_back(tex[0]);
						points.push_back(tex[1]);

						//���㷨����
						points.push_back(normal.x);
						points.push_back(normal.y);
						points.push_back(normal.z);
					}
				}

				//���������ʷ�
				QVector<Triangle> triangles = triangulation(bottom,isc);

				//��������
				for (int i = 0; i < triangles.size(); i++) {
					Triangle triangle = triangles[i];

					//���㷨����
					std::vector<float> tempPoints;
					for (int j = 0; j < 3; j++) {
						tempPoints.push_back(top[triangle.getPointIndex(j)].x);
						tempPoints.push_back(top[triangle.getPointIndex(j)].z);
						tempPoints.push_back(top[triangle.getPointIndex(j)].y);
					}

					glm::vec3 normal = computeNormal(tempPoints, isc);
					normal = -normal;

					//����
					for (int j = 0; j < 3; j++) {//���μ�����������
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


					//����
					for (int j = 0; j < 3; j++) {//���μ�����������
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

				unsigned int total = points.size() / 8;//��3 + 2 + 3��
				int sideNum = count / 2;//����ĸ���
				int topAndBottomNum = triangles.size() * 3 * 2;//����͵����ĸ���
				//unsigned int start  = total - sideNum * 4 - count;
				unsigned int start = total - sideNum * 4 - topAndBottomNum;
				
				//����index
				//1 ����
				for (int i = 0; i < sideNum; i++) {
					for (int j = 0; j < 2; j++) {
						for (int k = 0; k < 3; k++) {
							index.push_back(start + 4 * i + j + k);
						}
					}
				}

				//2 ����Ͷ���
				for (int i = 0; i < bottom.size() * 3 * 2; i+=3) {
					index.push_back(start + sideNum * 4 + i);
					index.push_back(start + sideNum * 4 + i + 1);
					index.push_back(start + sideNum * 4 + i + 2);
				}

				//����House
				houses.push_back(SingleHouse(count, rawPoint, houseXmin,houseXmax,houseZmin,houseZmax,houseYmin,houseYmax,triangles));

				/*��ģʽ*/
				//��������
				int linePointSize = 3;
				for (int i = 1; i <= count; i++) {
					lineHouseVertexs.push_back(rawPoint[linePointSize * (i - 1)]);
					lineHouseVertexs.push_back(rawPoint[linePointSize * (i - 1) + 1]);
					lineHouseVertexs.push_back(rawPoint[linePointSize * (i - 1) + 2]);
				}

				unsigned int total2 = lineHouseVertexs.size() / 3;
				int lineStart = total2 - count;
				//��������
				for (int i = 0; i < count; i+=2) {//���
					lineHouseIndexs.push_back(lineStart + i);
					lineHouseIndexs.push_back(lineStart + i + 1);
				}

				for (int i = 1; i < count; i += 2) {//����
					lineHouseIndexs.push_back(lineStart + i);
					lineHouseIndexs.push_back(lineStart + (i + 2) % count);
				}

				for (int i = 0; i < count; i += 2) {//�ױ�
					lineHouseIndexs.push_back(lineStart + i);
					lineHouseIndexs.push_back(lineStart + (i + 2) % count);
				}

				count = 0;
				house_num += 1;
				rawPoint.clear();
				continue;
			}
		}

		//��ȡһ������
		float x = std::stof(splitResult[0]);
		float y = std::stof(splitResult[1]);
		float zmax = std::stof(splitResult[2]);
		float zmin = std::stof(splitResult[3]);

		//����boundary
		if (house_num == 1 && count == 0) {//��ʼ��
			this->xmin = x;
			this->xmax = x;
			this->ymin = y;
			this->ymax = y;
			this->zmin = zmin;
			this->zmax = zmax;
		}
		else {//����
			this->xmin = x < this->xmin ? x : this->xmin;
			this->xmax = x > this->xmax ? x : this->xmax;

			this->ymin = y < this->ymin ? y : this->ymin;
			this->ymax = y > this->ymax ? y : this->ymax;

			this->zmin = zmin < this->zmin ? zmin : this->zmin;
			this->zmax = zmax > this->zmax ? zmax : this->zmax;
		}

		//����house��Χ��
		if (count == 0) {//��ʼ��
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
