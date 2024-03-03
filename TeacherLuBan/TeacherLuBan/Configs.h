#pragma once
#include <glm/glm.hpp> 


//鼠标信息
glm::vec2 lastmousePosition = glm::vec2(0, 0);//上一帧鼠标位置
glm::vec2 mousePositionDelta =glm::vec2(0);//鼠标位置增量
double mouseScrollDelta = 0.0f;//鼠标滚轮增量
bool isLeftMouseButtonRepeat = false;//鼠标左键是否按住
bool isLeftMouseButtonRelease = true;//鼠标左键是否抬起
bool isRightMouseButtonRepeat = false;//鼠标右键是否按住
bool isRightMouseButtonRelease = true;//鼠标右键是否抬起


unsigned int isTranslate = false;//是否在平移模型
unsigned int isRotate = false;//是否在旋转任意模型


unsigned int isScale = false;//是否在缩放任意模型


//键盘信息
glm::vec4 translateKeyDelta = glm::vec4(0);//平移按键增量
glm::vec4 rotateKeyDelta = glm::vec4(0);//旋转按键增量
glm::vec2 scaleKeyDelta = glm::vec2(0);//缩放按键增量



glm::mat4 view, projection;//视图、投影矩阵
glm::mat4 outlineModel;//描边模型矩阵
unsigned int diffuseMap;//漫反射贴图
unsigned int specularMap;//高光贴图
unsigned int backgroundMap;//背景贴图
glm::vec3 translateSum;//与世界坐标系原点的位移差


///vector<MyModel*> modelsbyAnimationSequence;//按照动画顺序排列的模型数组