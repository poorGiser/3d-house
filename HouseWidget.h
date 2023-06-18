#pragma once

#include <QtWidgets/QWidget>
#include "ui_HouseWidget.h"
#include <qopenglwidget.h>
#include <qopenglfunctions_3_3_core.h>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <vector>
#include "SingleHouse.h"
#include<qcheckbox.h>
#include <qpushbutton.h>
#include "Point3D.h"

enum class Type {
   HOUSE,LIGHT,LIGHT_HOUSE, DEPTH_MAP,SHADOW_HOUSE,SELECT_HOUSE,LINE_HOUSE,PATH_LINE,SKY_BOX
};

enum class Mode {
    CUBE,LINE
};

class HouseWidget : public QOpenGLWidget, QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

public:
    HouseWidget(QWidget *parent = nullptr);
    ~HouseWidget();

private:
    Ui::HouseWidgetClass ui;

    void vertexArray(Type type); //�������󶨶����������
    void bindVertexArray(Type type);//�󶨶����������
    void vertexBuffer(Type type);//�������󶨶��㻺�����
    void indexBuffer(Type type);//�������������������
    void createShader(const std::string& vertexShaderPath,const std::string& fragmentShaderPath,Type type);//����������ɫ��
    void bindShader(Type type);//����ɫ��
    void unBindShader();//�����ɫ��
    void createTexture(const std::string& path);//��·����������
    void activateTexture(int n);//����n������

    void loadCubemap(std::vector<const GLchar*> faces);

    void uniformMatrix4(const std::string& name,glm::mat4 martix4,Type type);//Ϊuniform������ֵ:����
    void uniform1i(const std::string& name, int i,Type type);//Ϊuniform������ֵ:int
    void uniform4f(const std::string& name, glm::vec4 vec, Type type);
    void uniform3f(const std::string& name, glm::vec3 vec, Type type);

    void computeLightVertexs(glm::vec3 lightPosition,float lightXSpan,float lightYSpan, float lightZSpan);//���ݹ�Դλ�úʹ�С�����Դ��������
    void getLightIndexs();//��Դ����index

    void initDepthFrameBuffer();//��ʼ��֡�������
    void initDepthMap();//�����������������ͼ
    void connectDFDM();//��������ͼ�󶨵�֡����������Ȼ���
    void unBindFrameBuffer();//���֡�������

    void computeSkyBoxVertexs(float xSpan,float ySpan,float ZSpan);//������պж�������
protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
private:
    Type currentShader;//��ǰHouse��ɫ��
    Mode currentMode;//��ǰģʽ


private:
    int currentHouseId;//��ǰ��ѯ��HouseId
    std::vector<unsigned int> selectHouseIndex;//��ǰ��ѯhouse��Index
    void computedSelectHouseIndex(int count);
private:
    QCheckBox* lightBox;
    QCheckBox* shadowBox;
    QCheckBox* roamBox;
    QCheckBox* cubeModelBox;
    QCheckBox* lineModelBox;
    QPushButton* selectBtn;
    QPushButton* clickSelectBtn;
    QPushButton* cancelBtn;
    QPushButton* pathSelectBtn;
    QPushButton* pathRoamBtn;
    QPushButton* cancelRoamBtn;

    void setActionState(bool state);
private:
    bool lightEnabled;//�Ƿ�����Դ
    bool shadowEnabled;//�Ƿ�����Ӱ
    bool roamEnabeld;//�Ƿ�����ά����

    glm::vec3 cameraPos; //���λ��
    glm::mat4 model; //houseģ�;���
    glm::mat4 view; //house��ͼ����
    glm::mat4 project; //houseͶӰ����


    GLuint m_vbo;//house���㻺�����
    GLuint m_vao;//house�����������
    GLuint m_ibo;//house���㻺�����
    GLuint shader;//house��ɫ������
    GLuint texture_id;//house�������

    std::vector<float> houseVertexs;//house���涥������
    std::vector<unsigned int> houseIndexs;//house������������
    
    std::vector<float> houseTopAndBottomVertexs;//house����͵�������
    std::vector<unsigned int> houseTopAndBottomIndexs;//house����͵�����������

    std::vector<SingleHouse> houses;//���廯��house��������

    GLuint skybox_texture_id;//��պ��������
    std::vector<float> skyboxVertexs;//��պж�������
    GLuint skyboxShader;//��պ���ɫ��
    GLuint skybox_vao;
    GLuint skybox_vbo;

    GLuint light_vao;
    GLuint light_vbo;
    GLuint light_ibo;
    GLuint lightShader;
    std::vector<float> lightVertexs;//��Դ��������
    std::vector<unsigned int> lightIndexs;//��Դ��������

    GLuint depthMap_fbo;//��Ӱ��ͼ֡�������
    GLuint depthMap;//��Ӱ��ͼ

    GLuint lightHouseShader; //��ӹ��պ��house��ɫ������

    GLuint depthMapShader;//�����ͼ����ɫ������

    GLuint shadowHouseShader; //�����Ӱ���house��ɫ������

    GLuint selectHouseShader;//��Ϣ��ѯ�����ɫ������

    GLuint pathPontShader;//·������ɫ��

    GLuint pathLineShader;//·������ɫ��

    QTimer* timer;
    int time_id;
private:
    std::vector<float> lineHouseVertexs;//��house��������
    std::vector<unsigned int> lineHouseIndexs;//��house��������
    GLuint line_house_vao;
    GLuint line_house_vbo;
    GLuint line_house_ibo;
    GLuint lineHouseShader;
    void updateLineHouseView();
    void updateLineHouseProj();
public slots:
    void lightAdmin();
    void shadowAdmin(int state);

    void keyPressEvent(QKeyEvent* event);//�����¼�
    void mouseMoveEvent(QMouseEvent* event) override;//����ƶ��¼�

    void roamAdmin();//����ģʽ����

    void selectHouse();//�����Ϣ��ѯ��ť
    void cancelSelect();//ȡ����Ϣ��ѯ

    void cubuModelAdmin();//��ģʽ
    void lineModelAdmin();//��ģʽ

    void wheelEvent(QWheelEvent* event) override; //�������¼�
    void mousePressEvent(QMouseEvent* event) override;//������¼�
    //void mouseDoubleClickEvent(QMouseEvent* event) override;//���˫���¼�

    void selectRoamPath();
    void startRoamPath();
    void pathRoamUpdate();
    void cancelPathRoam();

    void clickSelect();
private:
    bool pathRoam;
    bool showPath;
    std::vector<Point3D> pathPoints;
private:
    bool isClickSelect;
};
