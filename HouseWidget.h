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

    void vertexArray(Type type); //创建并绑定顶点数组对象
    void bindVertexArray(Type type);//绑定顶点数组对象
    void vertexBuffer(Type type);//创建并绑定顶点缓冲对象
    void indexBuffer(Type type);//创建并绑定索引缓冲对象
    void createShader(const std::string& vertexShaderPath,const std::string& fragmentShaderPath,Type type);//创建并绑定着色器
    void bindShader(Type type);//绑定着色器
    void unBindShader();//解绑着色器
    void createTexture(const std::string& path);//从路径创建纹理
    void activateTexture(int n);//激活n号纹理

    void loadCubemap(std::vector<const GLchar*> faces);

    void uniformMatrix4(const std::string& name,glm::mat4 martix4,Type type);//为uniform变量赋值:矩阵
    void uniform1i(const std::string& name, int i,Type type);//为uniform变量赋值:int
    void uniform4f(const std::string& name, glm::vec4 vec, Type type);
    void uniform3f(const std::string& name, glm::vec3 vec, Type type);

    void computeLightVertexs(glm::vec3 lightPosition,float lightXSpan,float lightYSpan, float lightZSpan);//根据光源位置和大小计算光源顶点数据
    void getLightIndexs();//光源顶点index

    void initDepthFrameBuffer();//初始化帧缓冲对象
    void initDepthMap();//创建并绑定深度纹理贴图
    void connectDFDM();//将纹理贴图绑定到帧缓冲对象的深度缓冲
    void unBindFrameBuffer();//解绑帧缓冲对象

    void computeSkyBoxVertexs(float xSpan,float ySpan,float ZSpan);//生成天空盒顶点数据
protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
private:
    Type currentShader;//当前House着色器
    Mode currentMode;//当前模式


private:
    int currentHouseId;//当前查询的HouseId
    std::vector<unsigned int> selectHouseIndex;//当前查询house的Index
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
    bool lightEnabled;//是否开启光源
    bool shadowEnabled;//是否开启阴影
    bool roamEnabeld;//是否开启三维漫游

    glm::vec3 cameraPos; //相机位置
    glm::mat4 model; //house模型矩阵
    glm::mat4 view; //house视图矩阵
    glm::mat4 project; //house投影矩阵


    GLuint m_vbo;//house顶点缓冲对象
    GLuint m_vao;//house顶点数组对象
    GLuint m_ibo;//house顶点缓冲对象
    GLuint shader;//house着色器对象
    GLuint texture_id;//house纹理对象

    std::vector<float> houseVertexs;//house侧面顶点数据
    std::vector<unsigned int> houseIndexs;//house侧面索引数据
    
    std::vector<float> houseTopAndBottomVertexs;//house顶面和底面数据
    std::vector<unsigned int> houseTopAndBottomIndexs;//house顶面和底面索引数据

    std::vector<SingleHouse> houses;//单体化的house顶点数据

    GLuint skybox_texture_id;//天空盒纹理对象
    std::vector<float> skyboxVertexs;//天空盒顶点数据
    GLuint skyboxShader;//天空盒着色器
    GLuint skybox_vao;
    GLuint skybox_vbo;

    GLuint light_vao;
    GLuint light_vbo;
    GLuint light_ibo;
    GLuint lightShader;
    std::vector<float> lightVertexs;//光源顶点数据
    std::vector<unsigned int> lightIndexs;//光源索引数据

    GLuint depthMap_fbo;//阴影贴图帧缓冲对象
    GLuint depthMap;//阴影贴图

    GLuint lightHouseShader; //添加光照后的house着色器对象

    GLuint depthMapShader;//深度贴图的着色器对象

    GLuint shadowHouseShader; //添加阴影后的house着色器对象

    GLuint selectHouseShader;//信息查询后的着色器对象

    GLuint pathPontShader;//路径点着色器

    GLuint pathLineShader;//路径线着色器

    QTimer* timer;
    int time_id;
private:
    std::vector<float> lineHouseVertexs;//线house顶点数据
    std::vector<unsigned int> lineHouseIndexs;//线house索引数据
    GLuint line_house_vao;
    GLuint line_house_vbo;
    GLuint line_house_ibo;
    GLuint lineHouseShader;
    void updateLineHouseView();
    void updateLineHouseProj();
public slots:
    void lightAdmin();
    void shadowAdmin(int state);

    void keyPressEvent(QKeyEvent* event);//键盘事件
    void mouseMoveEvent(QMouseEvent* event) override;//鼠标移动事件

    void roamAdmin();//漫游模式管理

    void selectHouse();//点击信息查询按钮
    void cancelSelect();//取消信息查询

    void cubuModelAdmin();//体模式
    void lineModelAdmin();//线模式

    void wheelEvent(QWheelEvent* event) override; //鼠标滚轮事件
    void mousePressEvent(QMouseEvent* event) override;//鼠标点击事件
    //void mouseDoubleClickEvent(QMouseEvent* event) override;//鼠标双击事件

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
