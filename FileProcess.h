#pragma once

#include <QObject>
#include <string>
#include <vector>
#include "SingleHouse.h"

class FileProcess  : public QObject
{
	Q_OBJECT

public:
	FileProcess(QObject *parent=nullptr);
	~FileProcess();

	std::string getContent(std::string path);//根据文件路径获取文件内容
	bool getHouseData(std::string path,std::vector<float>& points, std::vector<unsigned int>& index, std::vector<float>& houseTopAndBottomVertexs, std::vector<unsigned int>& houseTopAndBottomIndexs,std::vector<SingleHouse>& houses, std::vector<float>& lineHouseVertexs, std::vector<unsigned int>& lineHouseIndexs);//读取房屋数据
	float getXmin() { return xmin; }
	float getXmax() { return xmax; }

	float getYmin() { return ymin; }
	float getYmax() { return ymax; }

	float getZmin() { return zmin; }
	float getZmax() { return zmax; }
private:
	float xmin;
	float xmax;
	float ymin;
	float ymax;
	float zmin;
	float zmax;
};
