#pragma once
#include <glm/glm.hpp> 


//�����Ϣ
glm::vec2 lastmousePosition = glm::vec2(0, 0);//��һ֡���λ��
glm::vec2 mousePositionDelta =glm::vec2(0);//���λ������
double mouseScrollDelta = 0.0f;//����������
bool isLeftMouseButtonRepeat = false;//�������Ƿ�ס
bool isLeftMouseButtonRelease = true;//�������Ƿ�̧��
bool isRightMouseButtonRepeat = false;//����Ҽ��Ƿ�ס
bool isRightMouseButtonRelease = true;//����Ҽ��Ƿ�̧��


unsigned int isTranslate = false;//�Ƿ���ƽ��ģ��
unsigned int isRotate = false;//�Ƿ�����ת����ģ��


unsigned int isScale = false;//�Ƿ�����������ģ��


//������Ϣ
glm::vec4 translateKeyDelta = glm::vec4(0);//ƽ�ư�������
glm::vec4 rotateKeyDelta = glm::vec4(0);//��ת��������
glm::vec2 scaleKeyDelta = glm::vec2(0);//���Ű�������



glm::mat4 view, projection;//��ͼ��ͶӰ����
glm::mat4 outlineModel;//���ģ�;���
unsigned int diffuseMap;//��������ͼ
unsigned int specularMap;//�߹���ͼ
unsigned int backgroundMap;//������ͼ
glm::vec3 translateSum;//����������ϵԭ���λ�Ʋ�


///vector<MyModel*> modelsbyAnimationSequence;//���ն���˳�����е�ģ������