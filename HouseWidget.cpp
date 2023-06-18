#include "HouseWidget.h"
#include "FileProcess.h"
#include <iostream>
#include <qdebug.h>
#include <vector>
#include "stb_image.h"
#include <QVBoxLayout>
#include <qstring.h>
#include <qkeyevent>
#include <qmouseevent>
#include <qvector3d.h>
#include <QtMath>
#include <qmessagebox.h>
#include <qtoolbar.h>
#include <qinputdialog.h>
#include<qlabel.h>
#include<qtimer.h>
//#include "Triangle.h"

const unsigned int SHADOW_WIDTH = 1920, SHADOW_HEIGHT = 1080;
glm::vec3 cameraFront = glm::vec3(0.0, 0.0, -1.0);//���ָ��
float cameraSpeed;//����ƶ��ٶ�
glm::vec3 up = glm::vec3(0.0, 1.0, 0.0);//���������

float nearPlane = 10.0f;
float farPlane = 100.0f;

//z�Ḻ���heading��pitchֵ
float yaw = -90.0;
float pitch = 0.0;

//͸�Ӿ����fov�Ƕ�
float fov = 45.0;

//��Ļ��Ⱥ͸߶�
float screenWidth = 1920.0f;
float screenHeight = 1080.0f;

float roamSpeed = 50.0;
float frameRate = 60.0;

//std::vector<float> houseVertexs;

HouseWidget::HouseWidget(QWidget* parent)
	: QOpenGLWidget(parent), lightEnabled(false), shadowEnabled(false), lightHouseShader(0), depthMapShader(0), shadowHouseShader(0), currentShader(Type::HOUSE), roamEnabeld(false), currentHouseId(0), currentMode(Mode::CUBE), pathRoam(false)
{
	this->timer = NULL;
	ui.setupUi(this);
	this->resize(screenWidth, screenHeight);

	this->showPath = false;

	this->isClickSelect = false;

	//����������
	QToolBar* toolbar = new QToolBar(this);
	//toolbar->setStyleSheet("background-color:rgb(255,255,255);");//���ı�����ɫ
	ui.verticalLayout->addWidget(toolbar);

	QLabel* toolLabel = new QLabel(QStringLiteral("��������"));
	toolLabel->setStyleSheet("color:white;");
	toolbar->addWidget(toolLabel);

	lightBox = new QCheckBox(QStringLiteral("����"));
	lightBox->setStyleSheet("QCheckBox{color:rgb(0,0,139);font-weight:500;}");
	toolbar->addWidget(lightBox);
	QObject::connect(lightBox, SIGNAL(clicked()), this, SLOT(lightAdmin()));

	toolbar->addSeparator();

	shadowBox = new QCheckBox(QStringLiteral("��Ӱ"));
	shadowBox->setStyleSheet("QCheckBox{color:rgb(0,0,139);font-weight:500;}");
	toolbar->addWidget(shadowBox);
	shadowBox->setCheckable(false);
	QObject::connect(shadowBox, SIGNAL(stateChanged(int)), this, SLOT(shadowAdmin(int)));

	toolbar->addSeparator();

	roamBox = new QCheckBox(QStringLiteral("��ά����"));
	roamBox->setStyleSheet("QCheckBox{color:rgb(0,0,139);font-weight:500;}");
	toolbar->addWidget(roamBox);
	QObject::connect(roamBox, SIGNAL(clicked()), this, SLOT(roamAdmin()));

	toolbar->addSeparator();

	selectBtn = new QPushButton(QStringLiteral("����id��ѯ"));
	QObject::connect(selectBtn, SIGNAL(clicked()), this, SLOT(selectHouse()));
	toolbar->addWidget(selectBtn);

	clickSelectBtn = new QPushButton(QStringLiteral("�����ѯ"));
	QObject::connect(clickSelectBtn, SIGNAL(clicked()), this, SLOT(clickSelect()));
	toolbar->addWidget(clickSelectBtn);

	cancelBtn = new QPushButton(QStringLiteral("ȡ����ѯ"));
	QObject::connect(cancelBtn, SIGNAL(clicked()), this, SLOT(cancelSelect()));
	toolbar->addWidget(cancelBtn);

	toolbar->addSeparator();
	pathSelectBtn = new QPushButton(QStringLiteral("ѡ������·��"));
	QObject::connect(pathSelectBtn, SIGNAL(clicked()), this, SLOT(selectRoamPath()));

	pathRoamBtn = new QPushButton(QStringLiteral("��ʼ·������"));
	QObject::connect(pathRoamBtn, SIGNAL(clicked()), this, SLOT(startRoamPath()));
	pathRoamBtn->setEnabled(false);

	cancelRoamBtn = new QPushButton(QStringLiteral("ȡ��·������"));
	QObject::connect(cancelRoamBtn, SIGNAL(clicked()), this, SLOT(cancelPathRoam()));
	cancelRoamBtn->setEnabled(false);

	toolbar->addWidget(pathSelectBtn);
	toolbar->addWidget(pathRoamBtn);
	toolbar->addWidget(cancelRoamBtn);
	toolbar->addSeparator();

	cubeModelBox = new QCheckBox(QStringLiteral("��ģʽ"));
	QObject::connect(cubeModelBox, SIGNAL(clicked()), this, SLOT(cubuModelAdmin()));
	cubeModelBox->setChecked(true);
	cubeModelBox->setStyleSheet("QCheckBox{color:rgb(0,0,139);font-weight:500;}");
	toolbar->addWidget(cubeModelBox);

	lineModelBox = new QCheckBox(QStringLiteral("�߿�ģʽ"));
	QObject::connect(lineModelBox, SIGNAL(clicked()), this, SLOT(lineModelAdmin()));
	lineModelBox->setStyleSheet("QCheckBox{color:rgb(0,0,139);font-weight:500;}");
	toolbar->addWidget(lineModelBox);
	//toolbar->addWidget(checkBox2);
	//QObject::connect(ui.checkBox, SIGNAL(clicked()), this, SLOT(lightAdmin()));
	//ui.checkBox_2->setDisabled(true);
	//QObject::connect(ui.checkBox_2, SIGNAL(clicked()), this, SLOT(shadowAdmin()));
	//QObject::connect(ui.checkBox_3, SIGNAL(clicked()), this, SLOT(roamAdmin()));
}

HouseWidget::~HouseWidget()
{}

void HouseWidget::vertexBuffer(Type type)
{
	if (type == Type::HOUSE) {
		glGenBuffers(1, &this->m_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, this->m_vbo);
		glBufferData(GL_ARRAY_BUFFER, this->houseVertexs.size() * sizeof(GLfloat), &this->houseVertexs[0], GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
		glEnableVertexAttribArray(0);

		int offset = 0;
		offset += 3 * sizeof(GLfloat);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const void*)offset);
		glEnableVertexAttribArray(1);

		offset += 2 * sizeof(GLfloat);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const void*)offset);
		glEnableVertexAttribArray(2);
	}
	else if (type == Type::LIGHT) {
		glGenBuffers(1, &this->light_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, this->light_vbo);
		glBufferData(GL_ARRAY_BUFFER, this->lightVertexs.size() * sizeof(GLfloat), &this->lightVertexs[0], GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
		glEnableVertexAttribArray(0);
	}
	else if (type == Type::LINE_HOUSE) {
		glGenBuffers(1, &this->line_house_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, this->line_house_vbo);
		glBufferData(GL_ARRAY_BUFFER, this->lineHouseVertexs.size() * sizeof(GLfloat), &this->lineHouseVertexs[0], GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
		glEnableVertexAttribArray(0);
	}
	else if (type == Type::SKY_BOX) {
		glGenBuffers(1, &this->skybox_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, this->skybox_vbo);
		glBufferData(GL_ARRAY_BUFFER, this->skyboxVertexs.size() * sizeof(GLfloat), &this->skyboxVertexs[0], GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
		glEnableVertexAttribArray(0);
	}
}

void HouseWidget::vertexArray(Type type)
{
	if (type == Type::HOUSE) {
		glGenVertexArrays(1, &this->m_vao);
		glBindVertexArray(this->m_vao);
	}
	else if (type == Type::LIGHT) {
		glGenVertexArrays(1, &this->light_vao);
		glBindVertexArray(this->light_vao);
	}
	else if (type == Type::LINE_HOUSE) {
		glGenVertexArrays(1, &this->line_house_vao);
		glBindVertexArray(this->line_house_vao);
	}
	else if (type == Type::SKY_BOX) {
		glGenVertexArrays(1, &this->skybox_vao);
		glBindVertexArray(this->skybox_vao);
	}
}

void HouseWidget::bindVertexArray(Type type)
{
	if (type == Type::HOUSE) {
		glBindVertexArray(this->m_vao);
	}
	else if (type == Type::LIGHT) {
		glBindVertexArray(this->light_vao);
	}
	else if (type == Type::LINE_HOUSE) {
		glBindVertexArray(this->line_house_vao);
	}
	else if (type == Type::SKY_BOX) {
		glBindVertexArray(this->skybox_vao);
	}
}

void HouseWidget::indexBuffer(Type type)
{
	if (type == Type::HOUSE) {
		glGenBuffers(1, &this->m_ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->m_ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->houseIndexs.size() * sizeof(GLuint), &this->houseIndexs[0], GL_STATIC_DRAW);
	}
	else if (type == Type::LIGHT) {
		glGenBuffers(1, &this->light_ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->light_ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->lightIndexs.size() * sizeof(GLuint), &this->lightIndexs[0], GL_STATIC_DRAW);
	}
	else if (type == Type::LINE_HOUSE) {
		glGenBuffers(1, &this->line_house_ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->line_house_ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->lineHouseIndexs.size() * sizeof(GLuint), &this->lineHouseIndexs[0], GL_STATIC_DRAW);
	}
}

void HouseWidget::createShader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath, Type type)
{
	//��ȡ��ɫ������
	FileProcess fp;
	std::string vertexSource = fp.getContent(vertexShaderPath);
	std::string fragmentSource = fp.getContent(fragmentShaderPath);

	//������ɫ��
	GLuint vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);

	const char* vertexSrc = vertexSource.c_str();
	glShaderSource(vertexShader, 1, &vertexSrc, NULL);
	// ���붥����ɫ��
	glCompileShader(vertexShader);

	int  success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	// ����ķ���ֵΪsuccess��������ɹ�����������ķ�֧
	if (!success)
	{
		// ��ô�����Ϣ������������Ϣ����infoLog�н��д洢
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		// ��ӡ������Ϣ
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		return;
	}

	//ƬԪ��ɫ��
	GLuint fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	const char* fragmentSrc = fragmentSource.c_str();
	glShaderSource(fragmentShader, 1, &fragmentSrc, NULL);
	// ����ƬԪ��ɫ��
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	// ����ķ���ֵΪsuccess��������ɹ�����������ķ�֧
	if (!success)
	{
		// ��ô�����Ϣ������������Ϣ����infoLog�н��д洢
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		// ��ӡ������Ϣ
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		return;
	}

	// ������ɫ������
	if (type == Type::HOUSE) {
		this->shader = glCreateProgram();

		// ����ɫ�����ŵ�������
		glAttachShader(this->shader, vertexShader);
		glAttachShader(this->shader, fragmentShader);
		glLinkProgram(this->shader);
		glValidateProgram(this->shader);
		glUseProgram(this->shader);
	}
	else if (type == Type::LIGHT) {
		this->lightShader = glCreateProgram();

		// ����ɫ�����ŵ�������
		glAttachShader(this->lightShader, vertexShader);
		glAttachShader(this->lightShader, fragmentShader);
		glLinkProgram(this->lightShader);
		glValidateProgram(this->lightShader);
		glUseProgram(this->lightShader);
	}
	else if (type == Type::LIGHT_HOUSE) {
		this->lightHouseShader = glCreateProgram();

		// ����ɫ�����ŵ�������
		glAttachShader(this->lightHouseShader, vertexShader);
		glAttachShader(this->lightHouseShader, fragmentShader);
		glLinkProgram(this->lightHouseShader);
		glValidateProgram(this->lightHouseShader);
		glUseProgram(this->lightHouseShader);
	}
	else if (type == Type::DEPTH_MAP) {
		this->depthMapShader = glCreateProgram();
		// ����ɫ�����ŵ�������
		glAttachShader(this->depthMapShader, vertexShader);
		glAttachShader(this->depthMapShader, fragmentShader);
		glLinkProgram(this->depthMapShader);
		glValidateProgram(this->depthMapShader);
		glUseProgram(this->depthMapShader);
	}
	else if (type == Type::SHADOW_HOUSE) {
		this->shadowHouseShader = glCreateProgram();
		glAttachShader(this->shadowHouseShader, vertexShader);
		glAttachShader(this->shadowHouseShader, fragmentShader);
		glLinkProgram(this->shadowHouseShader);
		glValidateProgram(this->shadowHouseShader);
		glUseProgram(this->shadowHouseShader);
	}
	else if (type == Type::SELECT_HOUSE) {
		this->selectHouseShader = glCreateProgram();
		glAttachShader(this->selectHouseShader, vertexShader);
		glAttachShader(this->selectHouseShader, fragmentShader);
		glLinkProgram(this->selectHouseShader);
		glValidateProgram(this->selectHouseShader);
		glUseProgram(this->selectHouseShader);
	}
	else if (type == Type::LINE_HOUSE) {
		this->lineHouseShader = glCreateProgram();
		glAttachShader(this->lineHouseShader, vertexShader);
		glAttachShader(this->lineHouseShader, fragmentShader);
		glLinkProgram(this->lineHouseShader);
		glValidateProgram(this->lineHouseShader);
		glUseProgram(this->lineHouseShader);
	}
	else if (type == Type::PATH_LINE) {
		this->pathLineShader = glCreateProgram();
		glAttachShader(this->pathLineShader, vertexShader);
		glAttachShader(this->pathLineShader, fragmentShader);
		glLinkProgram(this->pathLineShader);
		glValidateProgram(this->pathLineShader);
		glUseProgram(this->pathLineShader);
	}
	else if (type == Type::SKY_BOX) {
		this->skyboxShader = glCreateProgram();
		glAttachShader(this->skyboxShader, vertexShader);
		glAttachShader(this->skyboxShader, fragmentShader);
		glLinkProgram(this->skyboxShader);
		glValidateProgram(this->skyboxShader);
		glUseProgram(this->skyboxShader);
	}

	// ���ڰ���ɫ���������ӵ���������Ժ�ɾ����ɫ������
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

void HouseWidget::bindShader(Type type)
{
	if (type == Type::HOUSE) {
		glUseProgram(this->shader);
	}
	else if (type == Type::LIGHT) {
		glUseProgram(this->lightShader);
	}
	else if (type == Type::LIGHT_HOUSE) {
		glUseProgram(this->lightHouseShader);
	}
	else if (type == Type::DEPTH_MAP) {
		glUseProgram(this->depthMapShader);
	}
	else if (type == Type::SHADOW_HOUSE) {
		glUseProgram(this->shadowHouseShader);
	}
	else if (type == Type::SELECT_HOUSE) {
		glUseProgram(this->selectHouseShader);
	}
	else if (type == Type::LINE_HOUSE) {
		glUseProgram(this->lineHouseShader);
	}
	else if (type == Type::PATH_LINE) {
		glUseProgram(this->pathLineShader);
	}
	else if (type == Type::SKY_BOX) {
		glUseProgram(this->skyboxShader);
	}
}

void HouseWidget::unBindShader()
{
	glUseProgram(0);
}

void HouseWidget::createTexture(const std::string& path)
{

	stbi_set_flip_vertically_on_load(1);
	int width, height, m_BPP;

	unsigned char* m_LocalBuffer;
	m_LocalBuffer = stbi_load(path.c_str(), &width, &height, &m_BPP, 4);

	glGenTextures(1, &this->texture_id);
	glBindTexture(GL_TEXTURE_2D, this->texture_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_LocalBuffer);

	if (m_LocalBuffer)
		stbi_image_free(m_LocalBuffer);
}

void HouseWidget::activateTexture(int n)
{
	glActiveTexture(GL_TEXTURE0 + n);
}

void HouseWidget::uniformMatrix4(const std::string& name, glm::mat4 martix4, Type type)
{
	if (type == Type::HOUSE) {
		GLuint location = glGetUniformLocation(this->shader, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, &martix4[0][0]);
	}
	else if (type == Type::LIGHT) {
		GLuint location = glGetUniformLocation(this->lightShader, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, &martix4[0][0]);
	}
	else if (type == Type::LIGHT_HOUSE) {
		GLuint location = glGetUniformLocation(this->lightHouseShader, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, &martix4[0][0]);
	}
	else if (type == Type::DEPTH_MAP) {
		GLuint location = glGetUniformLocation(this->depthMapShader, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, &martix4[0][0]);
	}
	else if (type == Type::SHADOW_HOUSE) {
		GLuint location = glGetUniformLocation(this->shadowHouseShader, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, &martix4[0][0]);
	}
	else if (type == Type::SELECT_HOUSE) {
		GLuint location = glGetUniformLocation(this->selectHouseShader, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, &martix4[0][0]);
	}
	else if (type == Type::LINE_HOUSE) {
		GLuint location = glGetUniformLocation(this->lineHouseShader, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, &martix4[0][0]);
	}
	else if (type == Type::PATH_LINE) {
		GLuint location = glGetUniformLocation(this->pathLineShader, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, &martix4[0][0]);
	}
	else if (type == Type::SKY_BOX) {
		GLuint location = glGetUniformLocation(this->skyboxShader, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, &martix4[0][0]);
	}
}

void HouseWidget::uniform1i(const std::string& name, int i, Type type)
{
	if (type == Type::HOUSE) {
		GLuint location = glGetUniformLocation(this->shader, name.c_str());
		glUniform1i(location, i);
	}
	else if (type == Type::LIGHT_HOUSE) {
		GLuint location = glGetUniformLocation(this->lightHouseShader, name.c_str());
		glUniform1i(location, i);
	}
	else if (type == Type::SHADOW_HOUSE)
	{
		GLuint location = glGetUniformLocation(this->shadowHouseShader, name.c_str());
		glUniform1i(location, i);
	}
	else if (type == Type::SKY_BOX) {
		GLuint location = glGetUniformLocation(this->skyboxShader, name.c_str());
		glUniform1i(location, i);
	}
}

void HouseWidget::uniform4f(const std::string& name, glm::vec4 vec, Type type)
{
	if (type == Type::LIGHT_HOUSE) {
		GLuint location = glGetUniformLocation(this->lightHouseShader, name.c_str());
		glUniform4f(location, vec.x, vec.y, vec.z, vec.w);
	}
	else if (type == Type::SHADOW_HOUSE) {
		GLuint location = glGetUniformLocation(this->shadowHouseShader, name.c_str());
		glUniform4f(location, vec.x, vec.y, vec.z, vec.w);
	}
}

void HouseWidget::uniform3f(const std::string& name, glm::vec3 vec, Type type)
{
	if (type == Type::LIGHT_HOUSE) {
		GLuint location = glGetUniformLocation(this->lightHouseShader, name.c_str());
		glUniform3f(location, vec.x, vec.y, vec.z);
	}
	else if (type == Type::SHADOW_HOUSE) {
		GLuint location = glGetUniformLocation(this->shadowHouseShader, name.c_str());
		glUniform3f(location, vec.x, vec.y, vec.z);
	}
}

void HouseWidget::computeLightVertexs(glm::vec3 lightPosition, float lightXSpan, float lightYSpan, float lightZSpan)
{
	//�������8������
	this->lightVertexs.push_back(lightPosition.x - lightXSpan / 2);
	this->lightVertexs.push_back(lightPosition.y + lightYSpan / 2);
	this->lightVertexs.push_back(lightPosition.z + lightZSpan / 2);

	this->lightVertexs.push_back(lightPosition.x + lightXSpan / 2);
	this->lightVertexs.push_back(lightPosition.y + lightYSpan / 2);
	this->lightVertexs.push_back(lightPosition.z + lightZSpan / 2);

	this->lightVertexs.push_back(lightPosition.x - lightXSpan / 2);
	this->lightVertexs.push_back(lightPosition.y - lightYSpan / 2);
	this->lightVertexs.push_back(lightPosition.z + lightZSpan / 2);

	this->lightVertexs.push_back(lightPosition.x + lightXSpan / 2);
	this->lightVertexs.push_back(lightPosition.y - lightYSpan / 2);
	this->lightVertexs.push_back(lightPosition.z + lightZSpan / 2);

	this->lightVertexs.push_back(lightPosition.x - lightXSpan / 2);
	this->lightVertexs.push_back(lightPosition.y + lightYSpan / 2);
	this->lightVertexs.push_back(lightPosition.z - lightZSpan / 2);

	this->lightVertexs.push_back(lightPosition.x + lightXSpan / 2);
	this->lightVertexs.push_back(lightPosition.y + lightYSpan / 2);
	this->lightVertexs.push_back(lightPosition.z - lightZSpan / 2);

	this->lightVertexs.push_back(lightPosition.x - lightXSpan / 2);
	this->lightVertexs.push_back(lightPosition.y - lightYSpan / 2);
	this->lightVertexs.push_back(lightPosition.z - lightZSpan / 2);

	this->lightVertexs.push_back(lightPosition.x + lightXSpan / 2);
	this->lightVertexs.push_back(lightPosition.y - lightYSpan / 2);
	this->lightVertexs.push_back(lightPosition.z - lightZSpan / 2);
}

void HouseWidget::getLightIndexs()
{
	this->lightIndexs = {
		0,1,2,
		1,2,3,
		1,3,7,
		1,7,5,
		5,7,6,
		5,6,4,
		0,4,6,
		0,6,2,
		0,1,4,
		4,1,5,
		2,6,3,
		6,3,7
	};
}

void HouseWidget::initDepthFrameBuffer()
{
	//makeCurrent();
	glGenFramebuffers(1, &this->depthMap_fbo);
	//qDebug() << depthMap_fbo << endl;
}

void HouseWidget::initDepthMap()
{
	//makeCurrent();
	glGenTextures(1, &this->depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	//qDebug() << this->depthMap << endl;
	//qDebug() << this->texture_id << endl;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void HouseWidget::connectDFDM()
{
	//makeCurrent();
	glBindFramebuffer(GL_FRAMEBUFFER, this->depthMap_fbo);
	// attach depth texture as FBO's depth buffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	// ���֡��������Ƿ�����
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		qDebug() << "error" << endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());

}

void HouseWidget::unBindFrameBuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void HouseWidget::computeSkyBoxVertexs(float xSpan, float ySpan, float zSpan)
{
	this->skyboxVertexs = std::vector<float>{
		-xSpan / 2,ySpan / 2,-zSpan / 2,
		-xSpan / 2,-ySpan / 2,-zSpan / 2,
		 xSpan / 2,-ySpan / 2,-zSpan / 2,
		 xSpan / 2,-ySpan / 2,-zSpan / 2,
		 xSpan / 2,ySpan / 2,-zSpan / 2,
		-xSpan / 2,ySpan / 2,-zSpan / 2,

		-xSpan / 2,-ySpan / 2,zSpan / 2,
		-xSpan / 2,-ySpan / 2,-zSpan / 2,
		-xSpan / 2,ySpan / 2,-zSpan / 2,
		-xSpan / 2,ySpan / 2,-zSpan / 2,
		-xSpan / 2,ySpan / 2,zSpan / 2,
		-xSpan / 2,-ySpan / 2,zSpan / 2,

		xSpan / 2,-ySpan / 2,-zSpan / 2,
		xSpan / 2,-ySpan / 2,zSpan / 2,
		xSpan / 2,ySpan / 2,zSpan / 2,
		xSpan / 2,ySpan / 2,zSpan / 2,
		xSpan / 2,ySpan / 2,-zSpan / 2,
		xSpan / 2,-ySpan / 2,-zSpan / 2,

		-xSpan / 2,-ySpan / 2,zSpan / 2,
		-xSpan / 2,ySpan / 2,zSpan / 2,
		xSpan / 2,ySpan / 2,zSpan / 2,
		xSpan / 2,ySpan / 2,zSpan / 2,
		xSpan / 2,-ySpan / 2,zSpan / 2,
		-xSpan / 2,-ySpan / 2,zSpan / 2,

		-xSpan / 2,ySpan / 2,-zSpan / 2,
		xSpan / 2,ySpan / 2,-zSpan / 2,
		xSpan / 2,ySpan / 2,zSpan / 2,
		xSpan / 2,ySpan / 2,zSpan / 2,
		-xSpan / 2,ySpan / 2,zSpan / 2,
		-xSpan / 2,ySpan / 2,-zSpan / 2,

		-xSpan / 2,-ySpan / 2,-zSpan / 2,
		-xSpan / 2,-ySpan / 2,zSpan / 2,
		xSpan / 2,-ySpan / 2,-zSpan / 2,
		xSpan / 2,-ySpan / 2,-zSpan / 2,
		-xSpan / 2,-ySpan / 2,zSpan / 2,
		xSpan / 2,-ySpan / 2,zSpan / 2,
	};
}

void HouseWidget::initializeGL() {
	this->initializeOpenGLFunctions();    //Ϊ��ǰ�����ĳ�ʼ���ṩOpenGL��������
	int num;
	glEnable(GL_DEPTH_TEST); //������Ȳ���
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);//�����ں�
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//��ȡHouse����
	FileProcess fp;

	bool result = fp.getHouseData("./res/data/house.txt", this->houseVertexs, this->houseIndexs, this->houseTopAndBottomVertexs, this->houseTopAndBottomIndexs, this->houses, this->lineHouseVertexs, this->lineHouseIndexs);
	if (!result) {
		QMessageBox::critical(this, "Error", "Read File Error!");
		exit(0);
	}

	//��ȡboundary
	//house���ݵ�Z����opengl����ϵ��Y�ᣬY����opengl����ϵ��Z��
	float xmin = fp.getXmin();
	float xmax = fp.getXmax();
	float ymin = fp.getZmin();
	float ymax = fp.getZmax();
	float zmin = fp.getYmin();
	float zmax = fp.getYmax();
	float xSpan = xmax - xmin;
	float ySpan = ymax - ymin;
	float zSpan = zmax - zmin;
	float center[3] = { (xmin + xmax) / 2,(ymin + ymax) / 2 ,(zmin + zmax) / 2 };

	cameraSpeed = zSpan / 100;

	//��������house��ɫ��
	this->createShader("./res/shader/vertex.shader", "./res/shader/fragment.shader", Type::HOUSE);

	//���MVP����
	//view����
	float dist = zSpan * 0.6;
	//his->cameraPos = glm::vec3(xSpan/2, ySpan/3, zSpan/2 + dist);
	this->cameraPos = glm::vec3(0.0, ySpan / 4, dist);
	// 
	//glm::vec3 viewCenter = glm::vec3(0.0,0.0,-1.0);
	glm::vec3 viewCenter = cameraPos + cameraFront;
	//glm::vec3 up = glm::vec3(0.0, 1.0, 0.0);
	this->view = glm::lookAt(cameraPos, viewCenter, up);
	//project����
	//glm::mat4 project = glm::ortho(xmin - xSpan/2, xmax + xSpan / 2, ymin - ySpan/4, ymax + ySpan / 2, -zmax, -zmin);
	//this->project = glm::ortho(-xSpan / 2 - xSpan / 2, xSpan / 2 + xSpan / 2, -ySpan / 2 - ySpan / 2, ySpan / 2 + ySpan / 2, -zSpan - zSpan, zSpan + zSpan);
	farPlane = dist + zSpan;
	project = glm::perspective(glm::radians(fov), screenWidth / screenHeight, nearPlane, farPlane);
	//model���󣺽�Houseƽ�Ƶ�����Ϊ(0,0,0.0,0.0)
	this->model = glm::translate(glm::mat4(1.0), glm::vec3(-center[0], -center[1], -center[2]));
	//glm::mat4 model = glm::mat4(1.0f);
	//mvp����
	glm::mat4 mvp = project * view * model;
	//this->uniformMatrix4("u_MVP", mvp, Type::HOUSE);
	this->uniformMatrix4("u_model", this->model, Type::HOUSE);
	this->uniformMatrix4("u_view", this->view, Type::HOUSE);
	this->uniformMatrix4("u_project", this->project, Type::HOUSE);

	//����
	//this->activateTexture(0);
	this->createTexture("./res/texture/house.jpg");
	this->activateTexture(0);
	glBindTexture(GL_TEXTURE_2D, this->texture_id);
	this->uniform1i("u_Texture", 0, Type::HOUSE);

	//�����������
	this->vertexArray(Type::HOUSE);
	//���㻺����
	this->vertexBuffer(Type::HOUSE);
	//����������
	this->indexBuffer(Type::HOUSE);

	//����ͬ���Ĳ���init��Դ
	//�����Դλ������������
	glm::vec3 lightPosition(xSpan, ySpan, 0); //��Դλ��

	float lightXSpan = xSpan / 10;
	float lightYSpan = ySpan / 10;
	float lightZSpan = zSpan / 10;
	this->computeLightVertexs(lightPosition, lightXSpan, lightYSpan, lightZSpan);
	this->getLightIndexs();
	//�󶨹�Դ��ɫ��
	this->unBindShader();
	this->createShader("./res/shader/light_vertex.shader", "./res/shader/light_fragment.shader", Type::LIGHT);
	glm::mat4 lightModel = glm::mat4(1.0);
	this->uniformMatrix4("u_model", lightModel, Type::LIGHT);
	this->uniformMatrix4("u_MVP", project * view * lightModel, Type::LIGHT);

	//����������󡢶��㻺����������������
	this->vertexArray(Type::LIGHT);
	this->vertexBuffer(Type::LIGHT);
	this->indexBuffer(Type::LIGHT);

	//��������house��ɫ��
	this->createShader("./res/shader/lighthouse_vertex.shader", "./res/shader/lighthouse_fragment.shader", Type::LIGHT_HOUSE);
	//��ɫ��������ֵ
	//this->uniformMatrix4("u_MVP", mvp, Type::LIGHT_HOUSE);
	this->uniformMatrix4("u_model", model, Type::LIGHT_HOUSE);
	this->uniformMatrix4("u_view", view, Type::LIGHT_HOUSE);
	this->uniformMatrix4("u_project", project, Type::LIGHT_HOUSE);

	this->uniform1i("u_Texture", 0, Type::LIGHT_HOUSE);
	this->uniform4f("u_light_color", glm::vec4(1.0, 1.0, 1.0, 1.0), Type::LIGHT_HOUSE);
	this->uniform3f("u_light_position", lightPosition, Type::LIGHT_HOUSE);
	//this->uniform3f("u_camera_position", glm::vec3(0.0,0.0,zmax + zSpan/10), Type::LIGHT_HOUSE);
	this->uniform3f("u_camera_position", cameraPos, Type::LIGHT_HOUSE);

	/*����֡���������������*/
	this->initDepthFrameBuffer();
	this->initDepthMap();
	this->connectDFDM();
	//this->unBindFrameBuffer();

	/*���������ͼ��ɫ��*/
	this->createShader("./res/shader/shadow_mapping_depth_vertex.shader", "./res/shader/shadow_mapping_depth_fragment.shader", Type::DEPTH_MAP);
	/*Ϊ�����ͼ��ɫ���е�lightSpace����ֵ*/
	glm::mat4 lightProjection, lightView;
	glm::mat4 lightSpaceMatrix;
	float near_plane = 0;
	float far_plane = xSpan * 2;
	lightProjection = glm::ortho(-zSpan, zSpan, -lightPosition.y - ySpan, ySpan, near_plane, far_plane);
	lightView = glm::lookAt(lightPosition, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
	lightSpaceMatrix = lightProjection * lightView;
	this->uniformMatrix4("lightSpaceMatrix", lightSpaceMatrix, Type::DEPTH_MAP);
	this->uniformMatrix4("u_model", model, Type::DEPTH_MAP);

	//����
	//this->bindShader(Type::HOUSE);
	//this->uniformMatrix4("u_view", lightView, Type::HOUSE);
	//this->uniformMatrix4("u_project", lightProjection, Type::HOUSE);

	this->unBindShader();
	/*���������Ӱ�����ɫ��*/
	this->createShader("./res/shader/shadow_house_vertex.shader", "./res/shader/shadow_house_fragment.shader", Type::SHADOW_HOUSE);
	/*��ɫ��������ֵ*/
	this->uniform1i("diffuseTexture", 0, Type::SHADOW_HOUSE);
	this->uniform1i("shadowMap", 1, Type::SHADOW_HOUSE);
	this->uniformMatrix4("u_project", this->project, Type::SHADOW_HOUSE);
	this->uniformMatrix4("u_view", this->view, Type::SHADOW_HOUSE);
	this->uniformMatrix4("u_model", this->model, Type::SHADOW_HOUSE);
	this->uniformMatrix4("lightSpaceMatrix", lightSpaceMatrix, Type::SHADOW_HOUSE);
	this->uniform3f("u_light_color", glm::vec3(1.0, 1.0, 1.0), Type::SHADOW_HOUSE);
	this->uniform3f("lightPos", lightPosition, Type::SHADOW_HOUSE);
	this->uniform3f("viewPos", cameraPos, Type::SHADOW_HOUSE);

	//select_house Shader
	this->createShader("./res/shader/select_house_vertex.shader", "./res/shader/select_house_fragment.shader", Type::SELECT_HOUSE);
	/*��ģʽHouse*/
	//vao
	this->vertexArray(Type::LINE_HOUSE);
	//vbo
	this->vertexBuffer(Type::LINE_HOUSE);
	//ibo
	this->indexBuffer(Type::LINE_HOUSE);
	//Shader
	this->createShader("./res/shader/line_house_vertex.shader", "./res/shader/line_house_fragment.shader", Type::LINE_HOUSE);

	//��ɫ��������ֵ
	this->uniformMatrix4("u_model", model, Type::LINE_HOUSE);
	this->uniformMatrix4("u_view", view, Type::LINE_HOUSE);
	this->uniformMatrix4("u_project", project, Type::LINE_HOUSE);

	//·������ɫ��
	this->createShader("./res/shader/path_point_vertex.shader", "./res/shader/path_point_fragment.shader", Type::PATH_LINE);
	//this->uniformMatrix4("u_model", model, Type::PATH_LINE);
	//this->uniformMatrix4("u_view", view, Type::PATH_LINE);
	//this->uniformMatrix4("u_project", project, Type::PATH_LINE);

	/*������պ�*/
	//��ʼ���������
	std::vector<const GLchar*> faces;
	faces.push_back("./res/texture/right.jpg");
	faces.push_back("./res/texture/left.jpg");
	faces.push_back("./res/texture/top.jpg");
	faces.push_back("./res/texture/bottom.jpg");
	faces.push_back("./res/texture/back.jpg");
	faces.push_back("./res/texture/front.jpg");
	this->loadCubemap(faces);

	//���㶥������
	this->computeSkyBoxVertexs(xSpan, ySpan, zSpan);
	//������ɫ��
	this->unBindShader();
	this->createShader("./res/shader/skybox_vertex.shader","./res/shader/skybox_fragment.shader",Type::SKY_BOX);
	//��ɫ��������ֵ
	//this->uniformMatrix4("u_view",view,Type::SKY_BOX);
	//this->uniformMatrix4("u_project", project, Type::SKY_BOX);

	this->vertexArray(Type::SKY_BOX);
	this->vertexBuffer(Type::SKY_BOX);
}
void HouseWidget::resizeGL(int w, int h) {
	glViewport(0, 0, w, h);    //�����ӿ�
}

void HouseWidget::paintGL() {

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);    //����
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);    //�����ɫ�������Ȼ���

	//��ǰģʽΪ��ģʽ
	if (this->currentMode == Mode::CUBE) {
		//���������Ӱ���Ȼ����Ӱ��ͼ
		if (this->shadowEnabled) {
			glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, this->depthMap_fbo);
			glClear(GL_DEPTH_BUFFER_BIT);
			this->bindShader(Type::DEPTH_MAP);
			this->bindVertexArray(Type::HOUSE);
			glDrawElements(GL_TRIANGLES, this->houseIndexs.size(), GL_UNSIGNED_INT, nullptr);
			glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		//������պ�
		glDepthMask(GL_FALSE);
		this->bindShader(Type::SKY_BOX);
		glm::mat4 skyView = glm::mat4(glm::mat3(view));
		this->uniformMatrix4("u_view", skyView, Type::SKY_BOX);
		this->uniformMatrix4("u_project", project, Type::SKY_BOX);
		// skybox cube
		this->bindVertexArray(Type::SKY_BOX);
		this->activateTexture(2);
		this->uniform1i("skybox", 2, Type::SKY_BOX);
		glBindTexture(GL_TEXTURE_CUBE_MAP, this->skybox_texture_id);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDepthMask(GL_TRUE);

		//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);    //����
		glViewport(0, 0, screenWidth, screenHeight);

		//������Դ
		//if (this->lightEnabled) {
		//	this->bindVertexArray(Type::LIGHT);
		//	this->bindShader(Type::LIGHT);
		//	glDrawElements(GL_TRIANGLES, this->lightIndexs.size(), GL_UNSIGNED_INT, nullptr); //���ƹ�Դ
		//}
		this->unBindShader();

		this->bindVertexArray(Type::HOUSE);
		this->bindShader(this->currentShader);
		this->uniformMatrix4("u_view", view, this->currentShader);
		//this->bindShader(this->currentShader);
		this->uniformMatrix4("u_project", project, this->currentShader);
		if (this->currentShader == Type::LIGHT_HOUSE) {
			//�������λ��
			this->uniform3f("u_camera_position", cameraPos, Type::LIGHT_HOUSE);
		}

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, this->texture_id);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		glDrawElements(GL_TRIANGLES, this->houseIndexs.size(), GL_UNSIGNED_INT, nullptr);
	}


	//��ǰģʽΪ��ģʽ
	if (this->currentMode == Mode::LINE) {
		//������պ�
		glDepthMask(GL_FALSE);
		this->bindShader(Type::SKY_BOX);
		glm::mat4 skyView = glm::mat4(glm::mat3(view));
		this->uniformMatrix4("u_view", skyView, Type::SKY_BOX);
		this->uniformMatrix4("u_project", project, Type::SKY_BOX);
		// skybox cube
		this->bindVertexArray(Type::SKY_BOX);
		this->activateTexture(2);
		this->uniform1i("skybox", 2, Type::SKY_BOX);
		glBindTexture(GL_TEXTURE_CUBE_MAP, this->skybox_texture_id);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDepthMask(GL_TRUE);

		this->unBindShader();
		this->bindVertexArray(Type::LINE_HOUSE);

		this->bindShader(Type::LINE_HOUSE);
		this->uniformMatrix4("u_view", view, Type::LINE_HOUSE);

		glDrawElements(GL_LINES, this->lineHouseIndexs.size(), GL_UNSIGNED_INT, nullptr);
	}

	//�����Ϣ��ѯ����Ҫ������ʾ��ѯ��House
	if (this->currentHouseId) {
		this->unBindShader();
		this->bindShader(Type::SELECT_HOUSE);
		this->uniformMatrix4("u_model", model, Type::SELECT_HOUSE);
		this->uniformMatrix4("u_view", view, Type::SELECT_HOUSE);
		this->uniformMatrix4("u_project", project, Type::SELECT_HOUSE);

		unsigned int select_house_vao;
		glGenVertexArrays(1, &select_house_vao);
		glBindVertexArray(select_house_vao);

		unsigned int select_house_vbo;
		glGenBuffers(1, &select_house_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, select_house_vbo);
		glBufferData(GL_ARRAY_BUFFER, this->houses[this->currentHouseId - 1].vertexs.size() * sizeof(GLfloat), &this->houses[this->currentHouseId - 1].vertexs[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
		glEnableVertexAttribArray(0);

		unsigned int select_house_ibo;
		glGenBuffers(1, &select_house_ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, select_house_ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->houses[this->currentHouseId - 1].indexs.size() * sizeof(GLuint), &this->houses[this->currentHouseId - 1].indexs[0], GL_STATIC_DRAW);

		glLineWidth(0.1f);
		glDrawElements(GL_TRIANGLES, this->houses[this->currentHouseId - 1].indexs.size(), GL_UNSIGNED_INT, nullptr);
	}

	//��������·��
	if (this->showPath && this->pathPoints.size() > 1) {
		std::vector<float> points;
		for (int i = 0; i < this->pathPoints.size(); i++) {
			points.push_back(this->pathPoints[i].x);
			points.push_back(this->pathPoints[i].y);
			points.push_back(this->pathPoints[i].z);
		}

		//������
		std::vector<unsigned int> pathLineIndexs;
		for (int i = 0; i < this->pathPoints.size() - 1; i++) {
			pathLineIndexs.push_back(i);
			pathLineIndexs.push_back(i + 1);
		}
		this->unBindShader();
		this->bindShader(Type::PATH_LINE);
		//this->uniformMatrix4("u_model", model, Type::PATH_LINE);
		this->uniformMatrix4("u_view", view, Type::PATH_LINE);
		this->uniformMatrix4("u_project", project, Type::PATH_LINE);

		unsigned int path_line_vao;
		glGenVertexArrays(1, &path_line_vao);
		glBindVertexArray(path_line_vao);

		unsigned int path_line_vbo;
		glGenBuffers(1, &path_line_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, path_line_vbo);
		glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(GLfloat), &points[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
		glEnableVertexAttribArray(0);

		unsigned int path_line_ibo;
		glGenBuffers(1, &path_line_ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, path_line_ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, pathLineIndexs.size() * sizeof(GLuint), &pathLineIndexs[0], GL_STATIC_DRAW);
		glLineWidth(5.0f);
		glDrawElements(GL_LINES, pathLineIndexs.size(), GL_UNSIGNED_INT, nullptr);
	}
}

void HouseWidget::shadowAdmin(int state)
{
	bool isShadow = this->shadowBox->isChecked();
	if (isShadow) {//�����Ӱ
		this->shadowEnabled = true;
		this->currentShader = Type::SHADOW_HOUSE;
		this->update();
	}
	else {//ȥ����Ӱ
		this->shadowEnabled = false;
		this->currentShader = Type::LIGHT_HOUSE;
		this->update();
	}
}

void HouseWidget::lightAdmin() {
	if (this->lightBox->isChecked() == true) {
		//��ӹ���
		this->lightEnabled = true;
		this->shadowBox->setCheckable(true);
		this->currentShader = Type::LIGHT_HOUSE;
		this->update();
	}
	else {
		//ȥ������
		this->shadowBox->setChecked(false);
		this->shadowBox->setCheckable(false);
		this->shadowEnabled = false;
		//this->shadowBox->setDisabled(true);
		this->lightEnabled = false;
		this->currentShader = Type::HOUSE;
		this->update();
	}
}

void HouseWidget::keyPressEvent(QKeyEvent* event) {
	makeCurrent();
	if (!this->roamEnabeld) return;
	if (event->key() == Qt::Key_W) {
		this->bindShader(this->currentShader);
		// ������W�������
		this->cameraPos += cameraSpeed * cameraFront;
		glm::vec3 viewCenter = this->cameraPos + cameraFront;
		this->view = glm::lookAt(this->cameraPos, viewCenter, up);
		this->uniformMatrix4("u_view", this->view, this->currentShader);
		this->update();
	}
	else if (event->key() == Qt::Key_S) {
		// ������S�������
		this->bindShader(this->currentShader);
		this->cameraPos -= cameraSpeed * cameraFront;
		glm::vec3 viewCenter = this->cameraPos + cameraFront;
		this->view = glm::lookAt(this->cameraPos, viewCenter, up);
		this->uniformMatrix4("u_view", this->view, this->currentShader);
		this->update();
	}
	else if (event->key() == Qt::Key_A) {
		// ������A�������
		this->bindShader(this->currentShader);
		this->cameraPos -= glm::normalize(glm::cross(cameraFront, up)) * cameraSpeed;
		glm::vec3 viewCenter = this->cameraPos + cameraFront;
		this->view = glm::lookAt(this->cameraPos, viewCenter, up);
		this->uniformMatrix4("u_view", this->view, this->currentShader);
		this->update();
	}
	else if (event->key() == Qt::Key_D) {
		// ������D�������
		this->bindShader(this->currentShader);
		this->cameraPos += glm::normalize(glm::cross(cameraFront, up)) * cameraSpeed;
		glm::vec3 viewCenter = this->cameraPos + cameraFront;
		this->view = glm::lookAt(this->cameraPos, viewCenter, up);
		this->uniformMatrix4("u_view", this->view, this->currentShader);
		this->update();
	}


	if (this->roamEnabeld) {
		if (event->key() == Qt::Key_Escape) {
			this->roamEnabeld = false;
			this->roamBox->setChecked(false);
		}
	}

	else {
		// ���������Ĵ���
		QWidget::keyPressEvent(event);
	}

	doneCurrent();
}

void HouseWidget::mouseMoveEvent(QMouseEvent* event) {
	if (!this->roamEnabeld) return;

	static bool firstMouse = true;
	static float lastX = width() / 2.0f;
	static float lastY = height() / 2.0f;

	if (firstMouse) {
		lastX = event->x();
		lastY = event->y();
		firstMouse = false;
	}

	float xOffset = event->x() - lastX;
	float yOffset = lastY - event->y();

	lastX = event->x();
	lastY = event->y();

	float sensitivity = 0.15f;
	xOffset *= sensitivity;
	yOffset *= sensitivity;

	yaw += xOffset;
	pitch += yOffset;

	// Լ�������Ƕȷ�Χ��-89��89��֮��
	if (pitch > 89.0f) {
		pitch = 89.0f;
	}
	if (pitch < -89.0f) {
		pitch = -89.0f;
	}

	// �����µ��ӽǷ�������
	QVector3D front;
	front.setX(cos(qDegreesToRadians(yaw)) * cos(qDegreesToRadians(pitch)));
	front.setY(sin(qDegreesToRadians(pitch)));
	front.setZ(sin(qDegreesToRadians(yaw)) * cos(qDegreesToRadians(pitch)));
	cameraFront = glm::vec3(front.normalized().x(), front.normalized().y(), front.normalized().z());

	glm::vec3 viewCenter = this->cameraPos + cameraFront;
	this->view = glm::lookAt(this->cameraPos, viewCenter, up);

	this->bindShader(this->currentShader);
	this->uniformMatrix4("u_view", this->view, this->currentShader);


	if (this->currentHouseId) {
		this->bindShader(Type::SELECT_HOUSE);
		this->uniformMatrix4("u_view", this->view, Type::SELECT_HOUSE);
	}

	this->updateLineHouseView();

	this->update();
}

void HouseWidget::roamAdmin() {
	bool isCheck = roamBox->isChecked();
	this->roamEnabeld = isCheck;
	if (isCheck) {
		QPoint center = geometry().center(); // ��ȡ�������ĵ�
		QCursor::setPos(center); // ������ƶ������ĵ�

		this->setMouseTracking(true);
	}
	else this->setMouseTracking(false);
}
void HouseWidget::selectHouse() {
	int id = QInputDialog::getInt(this, QStringLiteral("����Ի���"), QStringLiteral("������Ҫ��ѯ��HouseId:"), 1, 1, houses.size(), 1);
	//���ɶ���index
	this->currentHouseId = id;
	this->computedSelectHouseIndex(this->houses[id - 1].count);
	QMessageBox::information(this, QStringLiteral("��ʾ"), QStringLiteral("��ѯ�ɹ���"));
	this->cancelBtn->setEnabled(true);
	this->update();
}

void HouseWidget::computedSelectHouseIndex(int count) {
	if (this->houses[this->currentHouseId - 1].indexs.size() > 0) {
		return;
	}
	int sideNum = count / 2;
	for (int i = 0; i < sideNum; i++) {//����
		for (int j = 0; j < 2; j++) {
			for (int k = 0; k < 3; k++) {
				this->houses[this->currentHouseId - 1].indexs.push_back((i * 2 + j + k) % count);
			}
		}
	}

	QVector<Triangle> triangles = this->houses[this->currentHouseId - 1].triangles;

	//���� 2n+1
	for (auto triangle : triangles) {
		for (int i = 0; i < 3; i++) {
			this->houses[this->currentHouseId - 1].indexs.push_back(2 * triangle.getPointIndex(i) + 1);
		}
	}

	//���� 2n
	for (auto triangle : triangles) {
		for (int i = 0; i < 3; i++) {
			this->houses[this->currentHouseId - 1].indexs.push_back(2 * triangle.getPointIndex(i));
		}
	}
}

void HouseWidget::cancelSelect() {
	if (!currentHouseId) {
		QMessageBox::critical(this, QStringLiteral("��ʾ"), QStringLiteral("����δ��ʼ��ѯ��"));
		return;
	}
	this->isClickSelect = false;
	this->currentHouseId = 0;
	this->update();
}

void HouseWidget::cubuModelAdmin() {
	if (this->currentMode == Mode::CUBE) {
		this->cubeModelBox->setChecked(true);
		return;
	}

	this->cubeModelBox->setChecked(true);
	this->lineModelBox->setChecked(false);

	this->setActionState(true);

	this->currentMode = Mode::CUBE;
	this->update();
}

void HouseWidget::lineModelAdmin() {
	if (this->currentMode == Mode::LINE) {
		this->lineModelBox->setChecked(true);
		return;
	}

	this->lineModelBox->setChecked(true);
	this->cubeModelBox->setChecked(false);

	//����������ť
	this->setActionState(false);

	this->currentMode = Mode::LINE;
	this->update();
}

void HouseWidget::setActionState(bool state) {
	this->lightBox->setCheckable(state);
	this->shadowBox->setCheckable(state);
	//this->roamBox->setCheckable(state);
	//this->selectBtn->setEnabled(state);
	this->cancelBtn->setEnabled(state);
	//this->pathRoamBtn->setEnabled(state);
}

void HouseWidget::updateLineHouseView() {
	this->bindShader(Type::LINE_HOUSE);
	this->uniformMatrix4("u_view", this->view, Type::LIGHT_HOUSE);
}

void HouseWidget::updateLineHouseProj()
{
	this->bindShader(Type::LINE_HOUSE);
	this->uniformMatrix4("u_project", project, Type::LINE_HOUSE);
}

void HouseWidget::wheelEvent(QWheelEvent* event) {
	if (roamEnabeld) {
		// ��ȡ���ֵĹ�������͹�������
		int numDegrees = event->delta() / 8;
		int numSteps = numDegrees / 15;

		if (fov >= 1.0f && fov <= 45.0f)
			fov += numSteps;
		if (fov <= 1.0f)
			fov = 1.0f;
		if (fov >= 45.0f)
			fov = 45.0f;
		//���¾���
		this->project = glm::perspective(glm::radians(fov), screenWidth / screenHeight, nearPlane, farPlane);
		if (this->currentHouseId) {
			this->bindShader(Type::SELECT_HOUSE);
			this->uniformMatrix4("u_project", this->project, Type::SELECT_HOUSE);
		}
		this->updateLineHouseProj();
		update();
	}
}

void HouseWidget::mousePressEvent(QMouseEvent* event) {
	makeCurrent();
	if (this->pathRoam || this->isClickSelect) {
		// ��ȡ������ľֲ�λ��
		QPointF localPos = event->localPos();

		//��ȡ��Ļ����
		QPointF screenPos((localPos.x() / screenWidth) * 2 - 1.0f, 1.0f - (localPos.y() / screenHeight) * 2);

		// ��ȡ�����Ϣ
		GLfloat screenZ = 0.0f;
		glReadPixels(localPos.x(), screenHeight - localPos.y(), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &screenZ);
		//float linearDepth = (2.0 * nearPlane) / (farPlane + nearPlane - (2.0 * screenZ - 1.0) * (farPlane - nearPlane));
		screenZ = (screenZ - 0.5) * 2;

		// ת����׼���豸���굽��������
		glm::vec4 clipPos(screenPos.x(), screenPos.y(), screenZ, 1.0f);
		glm::mat4 inverseProjectionMatrix = glm::inverse(project);
		glm::mat4 inverseViewMatrix = glm::inverse(view);
		glm::mat4 inverseModelMatrix = glm::inverse(model);

		if (pathRoam) {
			//·������

			if (currentMode != Mode::LINE) {
				glm::vec4 eyePos = inverseViewMatrix * inverseProjectionMatrix * clipPos;
				eyePos /= eyePos.w;

				// ���յõ���eyePos��Ϊ���λ�õ���������
				float worldX = eyePos.x;
				float worldY = eyePos.y;
				float worldZ = eyePos.z;

				if (screenZ < 1.0) {
					//�����
					pathPoints.push_back(Point3D(worldX, worldY, worldZ));
					QMessageBox::information(this, QStringLiteral("��ʾ"), QStringLiteral("�ɹ����·���㣡"));

					//�������Ҽ���ر�
					if (event->button() == Qt::RightButton) {
						if (pathPoints.size() == 0) {
							QMessageBox::critical(this, QStringLiteral("��ʾ"), QStringLiteral("ֻ��һ��·���㣬�޷���ʼ���Σ������ѡ��·���㣡"));
							return;
						}
						QMessageBox::information(this, QStringLiteral("��ʾ"), QStringLiteral("·����ѡ����ϣ����Կ�ʼ���Σ�"));
						this->pathSelectBtn->setEnabled(false);
						this->pathRoamBtn->setEnabled(true);
						this->cancelRoamBtn->setEnabled(true);

						this->pathRoam = false;
					}
					update();
				}
			}
			else {
				QMessageBox::information(this, QStringLiteral("��ʾ"), QStringLiteral("�߿�ģʽ�޷�ѡ��·����"));
			}
		}

		if (this->isClickSelect) {
			glm::vec4 worldPos = inverseModelMatrix * inverseViewMatrix * inverseProjectionMatrix * clipPos;

			worldPos /= worldPos.w;

			float worldX = worldPos.x;
			float worldY = worldPos.y;
			float worldZ = worldPos.z;

			if (screenZ < 1.0) {//�㵽�˷���
				if (this->currentHouseId) {
					this->currentHouseId = 0;
					this->update();
				}
				//�жϵ��������һ������
				for (int j = 0; j < this->houses.size(); j++) {
					SingleHouse house = this->houses[j];
					//��ø÷��ݵİ�Χ��
					float house_xmin = house.boundings[0];
					float house_xmax = house.boundings[1];
					float house_ymin = house.boundings[2];
					float house_ymax = house.boundings[3];
					float house_zmin = house.boundings[4];
					float house_zmax = house.boundings[5];

					//�ж��Ƿ����÷���
					if (worldX >= house_xmin && worldX <= house_xmax && worldY >= house_ymin && worldY <= house_ymax && worldZ >= house_zmin && worldZ <= house_zmax) {
						this->currentHouseId = j + 1;
						this->computedSelectHouseIndex(this->houses[this->currentHouseId - 1].count);
						this->update();

						//�����÷�����Ϣ
						QString house_info = QString("id:,%1,height:,%2").arg(this->currentHouseId).arg(this->houses[this->currentHouseId - 1].boundings[3] - this->houses[this->currentHouseId - 1].boundings[2]);
						QMessageBox::information(this, QStringLiteral("��ʾ"), house_info);
						break;
					}
				}
			}
		}
	}


	doneCurrent();
}

void HouseWidget::selectRoamPath()//��ʼѡ������·��
{
	if (this->pathRoam) {
		QMessageBox::information(this, QStringLiteral("��ʾ"), QStringLiteral("�Ѿ�����ѡ������·����"));
		return;
	}

	if (this->currentMode == Mode::LINE) {
		QMessageBox::information(this, QStringLiteral("��ʾ"), QStringLiteral("�߿�ģʽ�޷�ѡ��·����"));
		return;
	}

	QMessageBox::information(this, QStringLiteral("��ʾ"), QStringLiteral("�����Կ�ʼѡ������·����"));
	this->setMouseTracking(true);
	this->pathRoam = true;
	this->showPath = true;
	this->pathSelectBtn->setEnabled(false);
	this->cancelRoamBtn->setEnabled(true);
}

void HouseWidget::startRoamPath()
{
	int swapInterval = this->format().swapInterval();
	frameRate = 60.0f / swapInterval;
	if (!this->timer) {
		timer = new QTimer(this);
		QObject::connect(timer, &QTimer::timeout, this, &HouseWidget::pathRoamUpdate);
		float flash = 1000.0 / frameRate;// ÿһ֡��ʱ��
		timer->setInterval(16);
	}
	timer->start();

	this->pathRoamBtn->setEnabled(false);
	this->cancelRoamBtn->setEnabled(true);
}

unsigned int i = 0;//�ڼ���·����
int count = 0;//��ǰ·��ǰ���Ĵ���
glm::vec3 pathCameraFront;
int totalCount;
float dx;
float dy;
float dz;
//glm::vec3 pathCameraPosition;
void HouseWidget::pathRoamUpdate()
{
	if (count == 0) {
		totalCount = qFloor((Point3D::getDis(pathPoints[i], pathPoints[i + 1]) / roamSpeed) * frameRate);  //��ǰ·����Ҫ���µ��ܴ���
		pathCameraFront = glm::vec3(pathPoints[i + 1].x - pathPoints[i].x, pathPoints[i + 1].y - pathPoints[i].y, pathPoints[i + 1].z - pathPoints[i].z);
		pathCameraFront = glm::normalize(pathCameraFront);
		dx = (pathPoints[i + 1].x - pathPoints[i].x) / totalCount;
		dy = (pathPoints[i + 1].y - pathPoints[i].y) / totalCount;
		dz = (pathPoints[i + 1].z - pathPoints[i].z) / totalCount;
	}
	cameraPos = glm::vec3(pathPoints[i].x + dx * count, pathPoints[i].y + dy * count + 2.0, pathPoints[i].z + dz * count);
	//this->cameraPos += 5.0f * pathCameraFront;
	//����view����
	this->view = glm::lookAt(cameraPos, cameraPos + pathCameraFront, up);
	//this->uniformMatrix4("u_view", this->view, this->currentShader);
	this->update();

	//���²���
	count++;
	if (count > totalCount) {
		i++;
		if (i == pathPoints.size() - 1) {//���ν���
			timer->stop();
			i = 0;
			count = 0;
			QMessageBox box(QMessageBox::Information, QStringLiteral("��ʾ"), QStringLiteral("�����ѽ�����Ҫ���¿�ʼ��"), QMessageBox::NoButton, this);
			box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
			box.setButtonText(QMessageBox::Yes, QStringLiteral("��"));
			box.setButtonText(QMessageBox::No, QStringLiteral("��"));
			int ret = box.exec();
			if (ret == QMessageBox::Yes) {
				timer->start();
			}
			else if (ret == QMessageBox::No) {
				this->cancelPathRoam();
			}
			return;
		}

		//����δ����
		totalCount = qFloor((Point3D::getDis(pathPoints[i], pathPoints[i + 1]) / roamSpeed) * frameRate);  //��ǰ·����Ҫ���µ��ܴ���
		count = 0;
	}
}

void HouseWidget::cancelPathRoam()//ȡ��·������
{
	//�����ʱ��
	if (this->timer) {
		this->timer->stop();
	}

	this->showPath = false;

	this->pathPoints.clear();

	i = 0;//�ڼ���·����
	count = 0;//��ǰ·��ǰ���Ĵ���

	this->pathSelectBtn->setEnabled(true);
	this->cancelRoamBtn->setEnabled(false);
	QMessageBox::information(this, QStringLiteral("��ʾ"), QStringLiteral("�����ѽ�����"));
}

void HouseWidget::clickSelect()
{
	if (this->currentMode == Mode::LINE) {
		QMessageBox::information(this, QStringLiteral("��ʾ"), QStringLiteral("�߿��޷�ͨ�����ѡ��"));
		return;
	}
	QMessageBox::information(this, QStringLiteral("��ʾ"), QStringLiteral("�����Ե��С�������Ի�ȡ����Ϣ��"));
	this->isClickSelect = true;
}

void HouseWidget::loadCubemap(std::vector<const GLchar*> faces)
{
	glGenTextures(1, &this->skybox_texture_id);

	int width, height;
	unsigned char* image;

	stbi_set_flip_vertically_on_load(0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, this->skybox_texture_id);
	for (GLuint i = 0; i < faces.size(); i++)
	{
		int width, height, m_BPP;
		image = stbi_load(faces[i], &width, &height, &m_BPP, 3);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		if (image) {
			stbi_image_free(image);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}
