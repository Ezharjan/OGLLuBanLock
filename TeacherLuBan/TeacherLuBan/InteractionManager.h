#pragma once
#include "LoadInAdvance.h"
#include "Configs.h"

using namespace std;
using namespace glm;



class InteractionManager
{
public:
	InteractionManager();
	~InteractionManager();
	void FrameBufferSizeCallback(GLFWwindow* window, int width, int height);
	void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
	void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
	void KeyBoardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

protected:
private:
};

void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}


//���λ�ûص�����
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
	mousePositionDelta = vec2(xpos - lastmousePosition.x, lastmousePosition.y - ypos);//ȡ�����λ�ò�ֵ
	lastmousePosition = vec2(xpos, ypos);//��¼��֡�����λ��
}

//��갴���ص�����
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	switch (action)
	{
	case GLFW_PRESS://��������һ֡����
		switch (button)
		{
		case GLFW_MOUSE_BUTTON_LEFT://������
			isLeftMouseButtonRelease = !(isTranslate = isLeftMouseButtonRepeat = true);
			break;
		case GLFW_MOUSE_BUTTON_RIGHT://����Ҽ�
			isRightMouseButtonRelease = !(isRotate = isRightMouseButtonRepeat = true);
			break;
		}
		break;
	case GLFW_RELEASE://̧������һ֡����
		switch (button)
		{
		case GLFW_MOUSE_BUTTON_LEFT://������
			isTranslate = isLeftMouseButtonRepeat = !(isLeftMouseButtonRelease = true);
			break;
		case GLFW_MOUSE_BUTTON_RIGHT://����Ҽ�
			isRotate = isRightMouseButtonRepeat = !(isRightMouseButtonRelease = true);
			break;
		}
		break;
	}
}

//�����ֻص�����
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	mouseScrollDelta = yoffset;
	isScale = (yoffset != 0);
}


//�����ص�����
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	glfwSetWindowShouldClose(window, key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE);//�����º�̧��Esc����رմ���
	if (action == GLFW_REPEAT)//����ǰ���ĳ������
	{
		translateKeyDelta = vec4(key == GLFW_KEY_W, key == GLFW_KEY_S, key == GLFW_KEY_A, key == GLFW_KEY_D);//WASD�����ƶ�
		rotateKeyDelta = vec4(key == GLFW_KEY_UP, key == GLFW_KEY_DOWN, key == GLFW_KEY_LEFT, key == GLFW_KEY_RIGHT);//�������ҿ�����ת
		scaleKeyDelta = vec2(key == GLFW_KEY_LEFT_BRACKET, key == GLFW_KEY_RIGHT_BRACKET);//[]��������

		//Ϊ�����������ӱ���
		{
			translateKeyDelta *= 10;
			rotateKeyDelta *= 10;
			scaleKeyDelta *= 10;
		}

		isTranslate = (key == GLFW_KEY_W || key == GLFW_KEY_S || key == GLFW_KEY_A || key == GLFW_KEY_D);
	}
	if (action == GLFW_RELEASE)
	{
		isTranslate = false;
	}
}






///////////////////////////////////////////////////////////////////////////
/*���������������ڽṹ�塭����������*/
struct Window
{
private:

	/*���ûص�����*/
	void SetCallback(GLFWwindow* window)
	{
		glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
		glfwSetMouseButtonCallback(window, MouseButtonCallback);
		glfwSetCursorPosCallback(window, CursorPosCallback);
		glfwSetScrollCallback(window, ScrollCallback);
		glfwSetKeyCallback(window, KeyCallback);
	}
public:
	GLFWwindow* window = NULL;
	const char* windowTitle = "LuBanLock";//��������
	float windowWidth = 1920.0f;//���ڿ��
	float windowHeight = 1080.0f;//���ڸ߶�
	//��ʼ������
	Window()
	{
		if (window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, NULL, NULL))//����һ�����ڶ��󣬲����ֱ�Ϊ���ߡ��������ƣ���������ʱ����
		{
			glfwMakeContextCurrent(window);//����������������Ϊ��ǰ�̵߳���������
			SetCallback(window);
		}
		else
		{
			cout << "���ڴ���ʧ�ܣ�";
			glfwTerminate();
		}
		lastmousePosition = vec2(0, 0);//��ʼ�����λ��
	}
};



/*������������ģ�ͽṹ�塭����������*/
struct MyModel
{
public:
	mat4 model;//ģ�;���

	vector<vector<float>> verticesData;//��������
	vector<vector<int>> dataIndices;//��������
	vector<vec3> animationPoint;//�������

	/*��Χ�нṹ��*/
	struct SurroundBox
	{
	private:
		vec3* initSurroundPoint = new vec3[8];//��ʼ��Χ��
		vec2* surroundPoint = new vec2[8];//��Χ��
		vec2* extramePoint = new vec2[4];//���˵�
		vec2* leftPoint = new vec2[4];//ʣ���
		enum RangePointType { xMin, xMax, yMin, yMax };//��Χ������

		//���Ƿ���ֱ���·�
		bool IsPointBelowLine(vec2 point, vec2 lineLeftPoint, vec2 lineRightPoint)
		{
			float A = (lineRightPoint.y) - (lineLeftPoint.y);
			float B = (lineLeftPoint.x) - (lineRightPoint.x);
			float C = (lineRightPoint.x * lineLeftPoint.y) - (lineLeftPoint.x * lineRightPoint.y);
			return ((A * point.x + B * point.y + C) > 0);
		}
	public:
		//��ʼ����Χ��
		SurroundBox(vector<vec3> points)
		{
			for (int i = 0; i < 8; i++)
			{
				initSurroundPoint[i] = points[i];
			}
		}
		//���°�Χ��
		void UpdateSurroundPoint(mat4 model, mat4 view, mat4 projection, Window window)
		{
			for (int i = 0; i < 8; i++)
			{
				vec4 tempPoint = (projection * view * model) * vec4(initSurroundPoint[i], 1);
				if (tempPoint.w != 0)
				{
					tempPoint /= tempPoint.w;
				}
				surroundPoint[i] = vec2(tempPoint.x * (window.windowWidth / 2) + (window.windowWidth / 2), tempPoint.y * -(window.windowHeight / 2) + (window.windowHeight / 2));//�õ���Χ������Ļ�ϵ�����
			}
		}

		//��ȡ���˵�
		void GetExtramePoint()
		{
			//��ʼ�����˵�
			for (size_t i = 0; i < 4; i++)
			{
				extramePoint[i] = surroundPoint[0];
			}

			//ͨ���Ƚϼ��㼫�˵�
			for (int i = 1; i < 8; i++)
			{
				if (extramePoint[RangePointType::xMin].x > surroundPoint[i].x)
				{
					extramePoint[RangePointType::xMin] = surroundPoint[i];
				}
				if (extramePoint[RangePointType::xMax].x < surroundPoint[i].x)
				{
					extramePoint[RangePointType::xMax] = surroundPoint[i];
				}
				if (extramePoint[RangePointType::yMin].y > surroundPoint[i].y)
				{
					extramePoint[RangePointType::yMin] = surroundPoint[i];
				}
				if (extramePoint[RangePointType::yMax].y < surroundPoint[i].y)
				{
					extramePoint[RangePointType::yMax] = surroundPoint[i];
				}
			}
		}

		/*��ȡʣ���*/
		void GetLeftPoint()
		{
			vector<vec2> tempPoint;//���ڳ�ʼ��ʣ������ʱ��
			bool isSame;//��Χ���Ƿ��뼫�˵�һ��
			//��¼��Χ���г�ȥ���˵�ĵ�
			for (size_t i = 0; i < 8; i++)//������Χ��
			{
				isSame = false;//�������µİ�Χ��
				for (size_t j = 0; j < 4; j++)//�������˵�
				{
					if (surroundPoint[i] == extramePoint[j])//���ĳ����Χ���ĳ�����˵�һ��
					{
						isSame = true;//�趨��Χ���뼫�˵�һ��
						break;
					}
				}
				if (!isSame)//������ĸ����˵㶼��һ��
				{
					tempPoint.push_back(surroundPoint[i]);//��¼�˵�
				}
			}

			//��ʼ��ʣ���
			for (size_t i = 0; i < 4; i++)
			{
				leftPoint[i] = tempPoint[0];
			}

			//ͨ���Ƚϼ���ʣ���
			for (int i = 1; i < 4; i++)
			{
				if (leftPoint[RangePointType::xMin].x > tempPoint[i].x)
				{
					leftPoint[RangePointType::xMin] = tempPoint[i];
				}
				if (leftPoint[RangePointType::xMax].x < tempPoint[i].x)
				{
					leftPoint[RangePointType::xMax] = tempPoint[i];
				}
				if (leftPoint[RangePointType::yMin].y > tempPoint[i].y)
				{
					leftPoint[RangePointType::yMin] = tempPoint[i];
				}
				if (leftPoint[RangePointType::yMax].y < tempPoint[i].y)
				{
					leftPoint[RangePointType::yMax] = tempPoint[i];
				}
			}
		}

		/*����Ƿ��ڰ�Χ����*/
		bool IsMouseSelect()
		{
			bool IsInExtramePointAera =
				!IsPointBelowLine(lastmousePosition, extramePoint[RangePointType::xMin], extramePoint[RangePointType::yMin])
				&& IsPointBelowLine(lastmousePosition, extramePoint[RangePointType::yMax], extramePoint[RangePointType::xMax])
				&& IsPointBelowLine(lastmousePosition, extramePoint[RangePointType::xMin], extramePoint[RangePointType::yMax])
				&& !IsPointBelowLine(lastmousePosition, extramePoint[RangePointType::yMin], extramePoint[RangePointType::xMax]);//�Ƿ��ڼ��˵㹹�ɵ�������

			bool IsInLeftPointAera =
				!IsPointBelowLine(lastmousePosition, leftPoint[RangePointType::xMin], leftPoint[RangePointType::yMin])
				&& IsPointBelowLine(lastmousePosition, leftPoint[RangePointType::yMax], leftPoint[RangePointType::xMax])
				&& IsPointBelowLine(lastmousePosition, leftPoint[RangePointType::xMin], leftPoint[RangePointType::yMax])
				&& !IsPointBelowLine(lastmousePosition, leftPoint[RangePointType::yMin], leftPoint[RangePointType::xMax]);//�Ƿ���ʣ��㹹�ɵ�������

			return (IsInExtramePointAera || IsInLeftPointAera);//ȡ����Ĳ���
		}
	};

	vector<SurroundBox> surroundBoxs;//��Χ��

	//��ʼ����������
	MyModel(vector<float> vertices_data, int trianglesNumber, vector<vector<vec3>> surroundPoints, vector<vec3>animationPoint)
	{
		verticesData.push_back(vertices_data);

		vector<int> indices_data;
		for (size_t i = 0; i < (trianglesNumber * 3); i++)
		{
			indices_data.push_back(i);
		}
		dataIndices.push_back(indices_data);
		InitVBOandVAO();
		InitEBO();

		for (unsigned int i = 0; i < surroundPoints.size(); i++)
		{
			for (size_t j = 0; j < surroundPoints[i].size(); j++)
			{
				surroundPoints[i][j] /= 30;
			}
			surroundBoxs.push_back(SurroundBox(surroundPoints[i]));//���ݰ�Χ�����ɰ�Χ��
		}

		this->animationPoint.push_back(vec3(0));
		this->animationPoint.insert(this->animationPoint.end(), animationPoint.begin(), animationPoint.end());//��ʼ��������
	}

	//��ʼ����������
	MyModel(vector<float> vertices_data, int trianglesNumber)
	{
		verticesData.push_back(vertices_data);

		vector<int> indices_data;
		for (size_t i = 0; i < (trianglesNumber * 3); i++)
		{
			indices_data.push_back(i);
		}
		dataIndices.push_back(indices_data);
		InitVBOandVAO();
		InitEBO();
	}

	/*��������(����λ�á����������)*/
	void ConfigAttribute(unsigned int location, unsigned int count, GLenum type, bool normalized, unsigned int stride, const void* pointer)
	{
		glVertexAttribPointer(location, count, type, normalized, stride, pointer);
		glEnableVertexAttribArray(location);
	}
	/*������*/
	void UnBindObject()
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);//���VBO��EBO
		glBindVertexArray(0);//���VAO
	}

	/*���ñ任����*/
	void SetTransform(vec3 translateDelta, vec2 RotateAngle, vec3 scaleDelta)
	{
		//������������ϵ������
		model = translate(mat4(1), -translateSum) * model;
		model = scale(mat4(1), scaleDelta) * model;
		model = translate(mat4(1), translateSum) * model;

		//������������ϵ����ת
		model = translate(mat4(1), -translateSum) * model;
		model = rotate(mat4(1), radians(RotateAngle.x), vec3(0, 1, 0)) * rotate(mat4(1), radians(RotateAngle.y), vec3(-1, 0, 0)) * model;//��ģ�;���ƽ�Ƶ���������ԭ������ת
		model = translate(mat4(1), translateSum) * model;

		//ƽ��
		model = translate(mat4(1), translateDelta) * model;
	}

	//���ö���
	void SetAnimation(float animationTime)
	{
		int animationIndex = 0;
		//ȷ����������
		while (animationTime >= ((animationIndex) * (1.0f / (animationPoint.size() - 1))))//�������ʱ�����ĳ����������Ӧ��ʱ��
		{
			++animationIndex;//������������
		}
		vec3 targetPosition =
			(
			(
				(animationTime - ((animationIndex - 1) * (1.0f / (animationPoint.size() - 1)))) / (1.0f / (animationPoint.size() - 1))
				)
				* (animationPoint[animationIndex] - animationPoint[animationIndex - 1])
				)
			+ animationPoint[animationIndex - 1];//��ȡ�ƶ���ֵ
		model = translate(model, targetPosition - position);//��ȡ����Ŀ��λ��
		position = targetPosition;//��¼Ŀ��λ��
	}

	/*���öԵ���������о۽������*/
	mat4 recordedModelMatrix;//��¼��ģ�;���
	void SetFocus(bool isFocus)
	{
		if (isFocus)
		{
			recordedModelMatrix = model;//��¼ģ�;���
			model = mat4(1);//����ģ�;���
		}
		else
		{
			model = recordedModelMatrix;//��ԭģ�;���
		}
	}

	/*����ģ�͡���ͼ��ͶӰ����*/
	void SetMVPMatrix(Shader shader)
	{
		shader.use();
		shader.setMat4("model", model);
		shader.setMat4("view", view);
		shader.setMat4("projection", projection);
	}

	/*ʹ�����������ƶ���*/
	void DrawObject(GLenum mode, unsigned int count, GLenum type, const void* indices)
	{
		glBindVertexArray(this->VAO);
		glDrawElements(mode, count, type, indices);
	}

	/*����Ƿ�ѡ������*/
	bool IsMouseSelect(Window window)
	{
		for (SurroundBox surroundBox : surroundBoxs)//�������а�Χ��
		{
			surroundBox.UpdateSurroundPoint(model, view, projection, window);
			surroundBox.GetExtramePoint();
			surroundBox.GetLeftPoint();
			if (surroundBox.IsMouseSelect())
			{
				return true;
			}
		}
		return false;
	}

	//���ñ任
	void ResetTransform()
	{
		model = mat4(1);
		translateSum = vec3(0);
	}

	/*�ͷſռ�*/
	void ReleaseSpace()
	{
		glDeleteBuffers(1, &VBO);
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &EBO);
	}

private:
	float* vertices_data;//��������
	int* indices_data;//��������
	enum class ObjectType { VAO, VBO, EBO };//��������
	unsigned int VBO;//���㻺�����
	unsigned int VAO;//�����������
	unsigned int EBO;//�����������

	vec3 position = vec3(0);//ģ��λ��
	/*��ʼ��VBO��VAO*/
	void InitVBOandVAO()
	{
		/*�����յĶ������ݴ洢����*/
		vertices_data = new float[verticesData[0].size()];
		for (size_t i = 0; i < verticesData[0].size(); i++)
		{
			vertices_data[i] = verticesData[0][i];
		}

		glGenBuffers(1, &VBO);//����VBO
		BindObject(ObjectType::VBO);
		glBufferData(GL_ARRAY_BUFFER, verticesData[0].size() * sizeof(float), vertices_data, GL_STATIC_DRAW);//���ö�������
		glGenVertexArrays(1, &VAO);//����VAO
		BindObject(ObjectType::VAO);
	}
	/*��ʼ��EBO*/
	void InitEBO()
	{
		/*�����յ��������ݴ洢����*/
		indices_data = new int[dataIndices[0].size()];
		for (size_t i = 0; i < dataIndices[0].size(); i++)
		{
			indices_data[i] = dataIndices[0][i];
		}
		glGenBuffers(1, &EBO);//����EBO
		BindObject(ObjectType::EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, dataIndices[0].size() * sizeof(int), indices_data, GL_STATIC_DRAW);//������������
	}
	/*�󶨶���*/
	void BindObject(ObjectType objectType)
	{
		switch (objectType)
		{
		case MyModel::ObjectType::VAO:
			glBindVertexArray(VAO);//��VAO
			break;
		case MyModel::ObjectType::VBO:
			glBindBuffer(GL_ARRAY_BUFFER, VBO);//��VBO
			break;
		case MyModel::ObjectType::EBO:
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);//��EBO
			break;
		}
	}
};
/*��������������������������������ȫ�ַ�����������������������������������*/
/*���������������������������þ��󡭡���������������������*/
//������ͼ����
void SetViewMatrix()
{
	view = translate(view, vec3(0, 0, -3));
}
//����ͶӰ����
void SetProjectionMatrix(Window window, bool isOrtho, float fov = 45.0f)
{
	projection = (isOrtho) ? ortho(-window.windowWidth / 1000, window.windowWidth / 1000, -window.windowHeight / 1000, window.windowHeight / 1000, 0.1f, 100.0f) : perspective(radians(fov), window.windowWidth / window.windowHeight, 0.1f, 100.0f);//�趨ͶӰ��ͼ��������͸��
}
/*������������������������ʱ�䡭����������������������*/
float deltaTime = 0;//ÿ��֡��ʱ���
//��ȡÿ��֮֡��ʱ���
void GetDeltaTime()
{
	float lastFrame = 0.0f;
	float currentTime = 0.0f;
	currentTime = glfwGetTime();//1.��¼��ǰ֡��Ӧʱ��
	deltaTime = currentTime - lastFrame;//3.�ӵڶ�֡��ʼ�������ǰ֡��ʱ���
	lastFrame = currentTime;//2.���浱ǰ֡ʱ�䣬��Ϊ��һ֡��ǰ֡ʱ��
}
/*��������������������������ʼ��������������������������*/
//��ʼ��GLFW
void InitGLFW()
{
	glfwInit();//��ʼ��GLFW
	/*glfwWindowHint��������GLFW����һ��������ѡ�����ƣ��ڶ���������ѡ���һ�����ε�ֵ*/
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//�趨���汾��Ϊ3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//�趨�ΰ汾��Ϊ3
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//����GLFWʹ�ú���ģʽ
}
//��ʼ��GLAD
void InitGLAD()
{
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))//glfwGetProcAddress�����ڼ���ϵͳ��ص�OpenGL����ָ���ַ�ĺ���(�������趨���߳�������֮�����)
	{
		cout << "��ʼ��GLADʧ��" << endl;
		glfwTerminate();
	}
	gladLoadGL();
}
//��ʼ��Imgui
void InitImgui(Window mainWindow)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\Dengb.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
	ImGui_ImplGlfw_InitForOpenGL(mainWindow.window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
}

vector<MyModel*> modelsbyAnimationSequence;//���ն���˳�����е�ģ������
//��ʼ������˳��
void InitAnimationSequence(vector<MyModel*> _models, vector<int> animation_sequence)
{
	for (size_t i = 0; i < _models.size(); i++)
	{
		modelsbyAnimationSequence.push_back(_models[animation_sequence[i]]);//����˳���¼����ģ��
	}
}
vector<MyModel*>models;//ģ������
//��ʼ��ģ������
void InitModels(vector<MyModel*>_models)
{
	for (MyModel* model : _models)
	{
		models.push_back(model);
	}
}
vec3 translateSumforRecord;//���ڼ�¼��ƽ��֮��
vector<mat4> modelsforRecord;//���ڼ�¼��ģ�;�������
//��ʼ����¼����
void InitRecordedModels()
{
	for (MyModel* model : models)
	{
		modelsforRecord.push_back(model->model);
	}
}
/*��������*/
unsigned int GenTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);//���ɲ���

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);//����ͼƬ
	if (data)//�������ͼƬ�ɹ�
	{
		GLenum format = (nrComponents == 3) ? GL_RGB : ((nrComponents == 4) ? GL_RGBA : GL_RED);//����ͼƬ��Ϣ�趨ɫ��ģʽ

		glBindTexture(GL_TEXTURE_2D, textureID);//��ͼƬ������
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);//��������
		glGenerateMipmap(GL_TEXTURE_2D);//���ɶ༶��Զ����

		/*���û��������ģʽ*/
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);//�ͷ�ͼƬ��Ϣ
	}
	else
	{
		cout << "�����޷�������·�����أ�" << path << endl;
		stbi_image_free(data);
	}

	return textureID;
}
/*�������ģ������*/
void ActiveandBindModelsTexture()
{
	glActiveTexture(GL_TEXTURE0);//��������������
	glBindTexture(GL_TEXTURE_2D, diffuseMap);//������������
	glActiveTexture(GL_TEXTURE1);//����߹�����
	glBindTexture(GL_TEXTURE_2D, specularMap);//�󶨸߹�����
}
/*������󶨱�������*/
void ActiveandBindBackgroundTexture()
{
	glActiveTexture(GL_TEXTURE0);//��������������
	glBindTexture(GL_TEXTURE_2D, backgroundMap);//������������
}
//��ʼ��ģ����ͼ
void InitModelTexture(Shader modelShader, char const* diffuseMapPath, char const* specularMapPath)
{
	stbi_set_flip_vertically_on_load(true);//�趨����ͼƬʱ��תy��
	diffuseMap = GenTexture(diffuseMapPath);//����·������������������ͼ
	specularMap = GenTexture(specularMapPath);//����·�����ɸ߹�������ͼ
	modelShader.use();
	modelShader.setInt("material.diffuse", 0);
	modelShader.setInt("material.specular", 1);
}
//��ʼ��ģ����ͼ
void InitModelTexture(Shader backgroundShader, char const* mapPath)
{
	stbi_set_flip_vertically_on_load(true);//�趨����ͼƬʱ��תy��
	backgroundMap = GenTexture(mapPath);//����·��������ͼ
	backgroundShader.use();
	backgroundShader.setInt("backgroundTexture", 0);
}
/*�������������������������ƹ⡭����������������������*/
vec3 lightPosition;//�ƹ�λ��
//���ɵ��Դ
void GenSpotLight(Shader modelShader, vec3 position, vec3 direction, float innerCutOffAngle, float outerCutOffAngle)
{
	modelShader.use();
	modelShader.setVec3("light.position", position);
	modelShader.setVec3("light.direction", direction);
	modelShader.setFloat("light.innerCutOff", cos(radians(innerCutOffAngle)));//���Ƕȵ�����ֵ����
	modelShader.setFloat("light.outerCutOff", cos(radians(outerCutOffAngle)));
	modelShader.setVec3("viewPosition", position);
}
//���ù�Դ����
void SetLightProperties(Shader modelShader, vec4 ambient, vec4 diffuse, vec4 specular, float shininess, float constant, float linear, float quadratic)
{
	modelShader.use();
	modelShader.setVec4("light.ambient", ambient);
	modelShader.setVec4("light.diffuse", diffuse);
	modelShader.setVec4("light.specular", specular);
	modelShader.setFloat("material.shininess", shininess);
	modelShader.setFloat("light.constant", constant);
	modelShader.setFloat("light.linear", linear);
	modelShader.setFloat("light.quadratic", quadratic);
}
/*����������������������������������������������������*/
//���ݸ����Ķ���ʱ��㲥�Ŷ���
void PlayAnimation(float animationTime, Shader modelShader)
{
	if (animationTime < 1)//ȷ��ʱ�䲻�ᳬ��1
	{
		int animationSequenceIndex = 0;//����˳������
		for (int i = 0; i < modelsbyAnimationSequence.size(); i++)
		{
			//��ȡ����˳������
			if (animationTime >= (animationSequenceIndex + 1) * (1.0f / modelsbyAnimationSequence.size()))
			{
				++animationSequenceIndex;
			}
		}
		for (int i = 0; i < modelsbyAnimationSequence.size(); i++)
		{
			if (i < animationSequenceIndex)
			{
				modelsbyAnimationSequence[i]->SetAnimation(0.9999f);
			}
			else if (i > animationSequenceIndex)
			{
				modelsbyAnimationSequence[i]->SetAnimation(0);
			}
		}
		animationTime -= (animationSequenceIndex) * (1.0f / modelsbyAnimationSequence.size());//��ʱ��ȷ�������������Ӧ�Ķ���ʱ��
		modelsbyAnimationSequence[animationSequenceIndex]->SetAnimation(animationTime / (1.0f / modelsbyAnimationSequence.size()));//���Ŷ�Ӧ����Ķ���
	}
}
/*�������������������������������á�����������������������*/
//���ò����Ƿ�����
void SetTestEnable(GLenum test, bool isEnable)
{
	if (isEnable)
	{
		glEnable(test);//��������
	}
	else
	{
		glDisable(test);//���ò���
	}
}
//����ģ��д��
void SetStencilWrite(bool isWritable)
{
	glStencilMask((isWritable) ? 0xFF : 0x00);//����ģ�����������ƶ�ģ�建���д��
}
/*������������������������ģ�͡�����������������������*/
//����ģ��
void ConfigModel(MyModel model)
{
	model.ConfigAttribute(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	model.ConfigAttribute(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	model.ConfigAttribute(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	model.UnBindObject();
}
//��������ģ�ͱ任
void SetAllModelsTransform(vec3 translateDelta, vec2 RotateAngle, vec3 scaleDelta)
{
	for (MyModel* model : models)
	{
		model->SetTransform(translateDelta, RotateAngle, scaleDelta);
	}
}

//�������
void DrawOutline(MyModel* model, Window mainWindow, Shader outlineShader, int trianglesNumber)
{
	outlineShader.use();
	outlineModel = model->model;
	outlineModel = scale(outlineModel, vec3(1.05f, 1.05f, 1.05f));
	outlineShader.setMat4("model", outlineModel);
	outlineShader.setMat4("view", view);
	outlineShader.setMat4("projection", projection);
	model->DrawObject(GL_TRIANGLES, trianglesNumber * 3, GL_UNSIGNED_INT, 0);
}

int outlineIndex = -1;//��Ҫ������������
bool isAnyModelFocused = false;//�Ƿ������屻�۽�
int focusedModelIndex = -1;//���۽�����������
bool isRecordedModelMartix = false;//�Ƿ��¼��ģ�;���
//����ģ��
void DrawModels(vector<MyModel*> models, vector<int> trianglesNumber, Window mainWindow, Shader modelShader, Shader outlineShader)
{
	if (!isAnyModelFocused)//���û���κ�ģ�ͱ��۽�
	{
		outlineIndex = -1;//�ٶ�û��������Ҫ�����

		for (size_t i = 0; i < (models.size()); i++)
		{
			models[i]->SetMVPMatrix(modelShader);

			if (!ImGui::IsAnyItemActive() && !ImGui::IsAnyWindowHovered() && models[i]->IsMouseSelect(mainWindow))//��ȥ�����ImGui�ϵ����
			{
				{
					SetTestEnable(GL_STENCIL_TEST, true);
					SetStencilWrite(true);
					glStencilFunc(GL_ALWAYS, 1, 0xFF);//����ģ����Ե�Ӱ������Ⱦ����
				}
				models[i]->DrawObject(GL_TRIANGLES, (3 * trianglesNumber[i]), GL_UNSIGNED_INT, 0);
				outlineIndex = i;//��¼��Ҫ����ߵ���������ֵ
				if (ImGui::IsMouseClicked(0))//�������������
				{
					focusedModelIndex = i;//��¼�����������
					return;
				}
			}
			else
			{
				{
					SetStencilWrite(false);
					SetTestEnable(GL_STENCIL_TEST, false);
				}
				models[i]->DrawObject(GL_TRIANGLES, (3 * trianglesNumber[i]), GL_UNSIGNED_INT, 0);
			}
		}

		if (outlineIndex != -1)//�����������Ҫ�����
		{
			{
				SetTestEnable(GL_STENCIL_TEST, true);
				glStencilFunc(GL_NOTEQUAL, 1, 0xFF);//��ģ��ֵ��Ϊ1����Ⱦ
				SetTestEnable(GL_DEPTH_TEST, false);
			}
			DrawOutline(models[outlineIndex], mainWindow, outlineShader, trianglesNumber[outlineIndex]);
			{
				SetStencilWrite(true);
				glClear(GL_STENCIL_BUFFER_BIT);//���ģ��ֵ
				SetStencilWrite(false);
			}
		}
	}
	else//������κ�ģ�ͱ��۽�
	{
		bool isNeedOutline = false;//�ٶ����岻��Ҫ�����

		models[focusedModelIndex]->SetMVPMatrix(modelShader);
		if (!ImGui::IsAnyItemActive() && !ImGui::IsAnyWindowHovered() && models[focusedModelIndex]->IsMouseSelect(mainWindow))
		{
			{
				SetTestEnable(GL_STENCIL_TEST, true);
				SetStencilWrite(true);
				glStencilFunc(GL_ALWAYS, 1, 0xFF);//����ģ����Ե�Ӱ������Ⱦ����
			}
			models[focusedModelIndex]->DrawObject(GL_TRIANGLES, (3 * trianglesNumber[focusedModelIndex]), GL_UNSIGNED_INT, 0);
			isNeedOutline = true;//������Ҫ�����
			if (ImGui::IsMouseClicked(0))//�������������
			{
				focusedModelIndex = -1;//�趨�����岻�ٱ��۽�
				return;
			}
		}
		else
		{
			{
				SetStencilWrite(false);
				SetTestEnable(GL_STENCIL_TEST, false);
			}
			models[focusedModelIndex]->DrawObject(GL_TRIANGLES, (3 * trianglesNumber[focusedModelIndex]), GL_UNSIGNED_INT, 0);
		}

		if (isNeedOutline)//���������Ҫ�����
		{
			{
				SetTestEnable(GL_STENCIL_TEST, true);
				glStencilFunc(GL_NOTEQUAL, 1, 0xFF);//��ģ��ֵ��Ϊ1����Ⱦ
				SetTestEnable(GL_DEPTH_TEST, false);
			}
			DrawOutline(models[focusedModelIndex], mainWindow, outlineShader, trianglesNumber[focusedModelIndex]);
			{
				SetStencilWrite(true);
				glClear(GL_STENCIL_BUFFER_BIT);//���ģ��ֵ
				SetStencilWrite(false);
			}
		}
	}
}

//���ÿ�ͨ��Ⱦ
void SetNPR(Shader modelShader, float NPRColorWeight[4], vec4 NPRColor[4])
{
	modelShader.use();
	modelShader.setFloat("npr.NPRColorWeight0", NPRColorWeight[0]);
	modelShader.setVec4("npr.NPRColor0", NPRColor[0]);
	modelShader.setFloat("npr.NPRColorWeight1", NPRColorWeight[1]);
	modelShader.setVec4("npr.NPRColor1", NPRColor[1]);
	modelShader.setFloat("npr.NPRColorWeight2", NPRColorWeight[2]);
	modelShader.setVec4("npr.NPRColor2", NPRColor[2]);
	modelShader.setFloat("npr.NPRColorWeight3", NPRColorWeight[3]);
	modelShader.setVec4("npr.NPRColor3", NPRColor[3]);
}

//��Ⱦ����
void RenderBackground(Shader backGroundShader, MyModel background)
{
	{
		SetTestEnable(GL_DEPTH_TEST, true);
		SetTestEnable(GL_STENCIL_TEST, false);
	}
	backGroundShader.use();
	backGroundShader.setMat4("view", view);
	backGroundShader.setMat4("projection", projection);

	background.DrawObject(GL_TRIANGLES, 3 * 2, GL_UNSIGNED_INT, 0);
}
/*��������������������������Ⱦ����������������������������*/
//������л���
void ClearAllBuffer()
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);//���ô��������ɫ
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);//�����ɫ�������ģ�建��
}
/*��������������������������������ImGUI���á�������������������������������*/
//����Imgui��֡
void SetImguiNewFrame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

int isOrtho = false;//�Ƿ�Ϊ����ͶӰ
float fov = 45.0f;//͸�ӽǶ�
static char backgroundPath[64] = "Resources/Textures/Background/02.png";//����ͼƬ·��
static const char* modelMaps[9][2] = //ģ����ͼ
{
	{ "Resources/Textures/Diffuse/01.png","Resources/Textures/Specular/01.png" },
	{ "Resources/Textures/Diffuse/02.png","Resources/Textures/Specular/02.png" },
	{ "Resources/Textures/Diffuse/03.png", "Resources/Textures/Specular/03.png" },
	{ "Resources/Textures/Diffuse/04.png", "Resources/Textures/Specular/04.png" },
	{ "Resources/Textures/Diffuse/05.png", "Resources/Textures/Specular/05.png" },
	{ "Resources/Textures/Diffuse/06.png", "Resources/Textures/Specular/06.png" },
	{ "Resources/Textures/Diffuse/07.png", "Resources/Textures/Specular/07.png" },
	{ "Resources/Textures/Diffuse/08.png", "Resources/Textures/Specular/08.png" },
	{ "Resources/Textures/Diffuse/09.png", "Resources/Textures/Specular/09.png" }
};
static int modelMapIndex = 0;//ģ����ͼ����
float lightDepth = 3;//�ƹ����
float innerCutoffAngle = 12.5f;//�ڹ���
float outerCutoffAngle = 17.5f;//�����
ImVec4 ambient(0.1, 0.1, 0.1, 1);//������
ImVec4 diffuse(0.8f, 0.8f, 0.8f, 1.0f);//�������
ImVec4 specular(1.0f, 1.0f, 1.0f, 1.0f);//�߹�
int shininess = 2;//�����
bool isLockLightPoiiton = false;//�Ƿ������ƹ�λ��
bool isAnimationPlaying = false;//�����Ƿ��ڲ���
float animationTime = 0.0f;//�ܶ���ʱ����
bool isUseNPR = false;//�Ƿ�ʹ��NPR
float NPRColorWeight[4] = { 0.75f,0.5f,0.25f,0.0f };//NPC��ɫȨ��
ImVec4 NPRColor[4] = { ImVec4(1,0.5,0.5,1),ImVec4(0.6,0.3,0.3,1), ImVec4(0.4,0.2,0.2,1), ImVec4(0.2,0.1,0.1,1) };//NPR��ɫ
//����Imgui������
void GenImguiMainWindow(Shader modelShader, Shader backGroundShader)
{
	ImGui::Begin(u8"��ӭʹ��³������ѧ��ʾ����", NULL, ImGuiWindowFlags_NoMove);
	ImGui::SetWindowPos(ImVec2(0, 0));
	ImGui::Spacing();
	if (ImGui::CollapsingHeader(u8"���"))
	{
		ImGui::Indent();
		ImGui::Bullet(); ImGui::TextWrapped(u8"��ӭ���������ڴ˳�������ᵽ³�������������ڣ�");
		ImGui::Bullet(); ImGui::TextWrapped(u8"������ͨ�����ͼ��̶�ģ�ͽ��н�����ʹ�ô�UI��������и��Ի�����");
		ImGui::Unindent();
	}
	ImGui::Spacing();
	if (ImGui::CollapsingHeader(u8"����˵��"))
	{
		{
			ImGui::Indent();
			if (ImGui::TreeNode(u8"��UI����"))
			{
				ImGui::Indent();
				if (ImGui::TreeNode(u8"���"))
				{
					ImGui::Indent();
					ImGui::Bullet(); ImGui::TextWrapped(u8"�����λ�ڷ�ͼ�ν���հ�����ʱ��������ʹ��������������н���");

					ImGui::Indent();
					if (ImGui::TreeNode(u8"ƽ��"))
					{
						ImGui::Bullet(); ImGui::TextWrapped(u8"�϶���������ƽ������");
						ImGui::TreePop();
					}
					if (ImGui::TreeNode(u8"��ת"))
					{
						ImGui::Bullet(); ImGui::TextWrapped(u8"�϶�����Ҽ���ƽ������");
						ImGui::TreePop();
					}
					if (ImGui::TreeNode(u8"����"))
					{
						ImGui::Bullet(); ImGui::TextWrapped(u8"ʹ������������������");
						ImGui::TreePop();
					}
					if (ImGui::TreeNode(u8"�۽�ģʽ"))
					{
						ImGui::Bullet(); ImGui::TextWrapped(u8"��������ģ���ϻ��������ʾ");
						ImGui::Bullet(); ImGui::TextWrapped(u8"��ʱ�������������Խ���۽�ģʽ�������鿴ѡ�е�����");
						ImGui::TreePop();
					}
					ImGui::Unindent();
					ImGui::Unindent();
					ImGui::TreePop();
				}
				if (ImGui::TreeNode(u8"����"))
				{
					ImGui::Indent();
					ImGui::Bullet(); ImGui::TextWrapped(u8"ʹ�ü��̽���ʱ����ȷ������Ӣ������״̬");

					ImGui::Indent();
					if (ImGui::TreeNode(u8"ƽ��"))
					{
						ImGui::Bullet(); ImGui::TextWrapped(u8"ʹ��WASD����ƽ������");
						ImGui::TreePop();
					}
					if (ImGui::TreeNode(u8"��ת"))
					{
						ImGui::Bullet(); ImGui::TextWrapped(u8"ʹ�÷��������ת����");
						ImGui::TreePop();
					}
					if (ImGui::TreeNode(u8"����"))
					{
						ImGui::Bullet(); ImGui::TextWrapped(u8"ʹ��\"[ ]\"������������");
						ImGui::TreePop();
					}
					ImGui::Unindent();
					ImGui::Unindent();
					ImGui::TreePop();
				}
				ImGui::Unindent();
				ImGui::TreePop();
			}
			ImGui::Unindent();
		}

		{
			ImGui::Indent();
			if (ImGui::TreeNode(u8"UI����"))
			{
				ImGui::Indent();
				ImGui::Bullet(); ImGui::TextWrapped(u8"ʹ�������е�����϶����������̽����������");
				ImGui::Bullet(); ImGui::TextWrapped(u8"��סCtrl���������������ʹ�ü���������ֵ");
				ImGui::Unindent();
				ImGui::TreePop();
			}
			ImGui::Unindent();
		}
	}
	ImGui::Spacing();
	if (ImGui::CollapsingHeader(u8"ͶӰ����"))
	{
		ImGui::Indent();
		ImGui::RadioButton(u8"͸��ͶӰ", &isOrtho, 0);
		if (!isOrtho)
		{
			ImGui::SameLine(); ImGui::SliderFloat(u8"�ӳ�(FOV)", &fov, 0, 45);
		}
		ImGui::RadioButton(u8"����ͶӰ", &isOrtho, 1);
		ImGui::Unindent();
	}
	ImGui::Spacing();
	if (ImGui::CollapsingHeader(u8"��������"))
	{
		ImGui::Indent();
		if (ImGui::InputTextWithHint(u8"����ͼƬ·��", u8"ע�ⲻ��ʹ������·������Ҫȥ��""�����»س�����ȷ��", backgroundPath, 64, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			InitModelTexture(backGroundShader, backgroundPath);
		}
		ImGui::Unindent();
	}
	ImGui::Spacing();
	if (ImGui::CollapsingHeader(u8"ģ������"))
	{
		ImGui::Indent();
		if (ImGui::Combo(u8"ѡ��ģ����ͼ", &modelMapIndex, " 01\0 02\0 03\0 04\0 05\0 06\0 07\0 08\0 09\0"))
		{
			InitModelTexture(modelShader, modelMaps[modelMapIndex][0], modelMaps[modelMapIndex][1]);
		}
		ImGui::Checkbox(u8"�Ƿ�����ͨ��Ⱦ", &isUseNPR);
		modelShader.use();
		modelShader.setBool("npr.isUseNPR", isUseNPR);
		if (isUseNPR)
		{
			ImGui::SliderFloat(u8"����0", &NPRColorWeight[0], NPRColorWeight[1], 1);
			ImGui::ColorEdit4(u8"��ɫ0", (float*)&NPRColor[0], ImGuiColorEditFlags_Float);
			ImGui::Separator();
			ImGui::SliderFloat(u8"����1", &NPRColorWeight[1], NPRColorWeight[2], NPRColorWeight[0]);
			ImGui::ColorEdit4(u8"��ɫ1", (float*)&NPRColor[1], ImGuiColorEditFlags_Float);
			ImGui::Separator();
			ImGui::SliderFloat(u8"����2", &NPRColorWeight[2], NPRColorWeight[3], NPRColorWeight[1]);
			ImGui::ColorEdit4(u8"��ɫ2", (float*)&NPRColor[2], ImGuiColorEditFlags_Float);
			ImGui::Separator();
			ImGui::SliderFloat(u8"����3", &NPRColorWeight[3], 0, NPRColorWeight[2]);
			ImGui::ColorEdit4(u8"��ɫ3", (float*)&NPRColor[3], ImGuiColorEditFlags_Float);
		}
		ImGui::Unindent();
	}
	ImGui::Spacing();
	if (ImGui::CollapsingHeader(u8"�ƹ�����"))
	{
		ImGui::Indent();
		ImGui::SliderFloat(u8"�ƹ����", &lightDepth, 0, 10);
		if (!isUseNPR)
		{
			ImGui::SliderFloat(u8"�ڹ���", &innerCutoffAngle, 0, outerCutoffAngle);
			ImGui::SliderFloat(u8"�����", &outerCutoffAngle, innerCutoffAngle, 89);
			ImGui::Separator();
			ImGui::ColorEdit4(u8"������", (float*)&ambient, ImGuiColorEditFlags_Float);
			ImGui::ColorEdit4(u8"�������", (float*)&diffuse, ImGuiColorEditFlags_Float);
			ImGui::ColorEdit4(u8"�����", (float*)&specular, ImGuiColorEditFlags_Float);
			ImGui::Text("2^"); ImGui::SameLine(); ImGui::SliderInt(u8"�����", &shininess, 0, 10);
			ImGui::Separator();
		}
		ImGui::Checkbox(u8"�����ƹ�λ�õ���Ļ����", &isLockLightPoiiton);
		if (isLockLightPoiiton)
		{
			lightPosition = vec3(0, 0, lightDepth);
		}
		ImGui::Unindent();
	}
	ImGui::Spacing();
	if (ImGui::CollapsingHeader(u8"��������"))
	{
		ImGui::Indent();
		if (ImGui::Button(u8"����"))
		{
			animationTime = 0;
		}
		ImGui::SameLine(); isAnimationPlaying = (!isAnimationPlaying) ? ImGui::Button(u8"����") : !(ImGui::Button(u8"��ͣ"));
		if (isAnimationPlaying && animationTime < 1)
		{
			animationTime += (deltaTime / 50000);
		}
		ImGui::SameLine(); float multipliedAnimationTime = animationTime * 100; ImGui::SliderFloat(u8"����������", &multipliedAnimationTime, 0, 100, "%.3f %%"); animationTime = multipliedAnimationTime / 100;
		ImGui::Unindent();
	}
	ImGui::End();
}
//��ȾImgui
void RenderImgui()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
/*�����������������������������������á�������������������������������*/
//�����������
void ResetMouseDelta()
{
	mouseScrollDelta = 0.0f;
	mousePositionDelta = vec2(0);
}
//���ü�������
void ResetKeyDelta()
{
	translateKeyDelta = vec4(0);//ƽ�ư�������
	rotateKeyDelta = vec4(0);//��ת��������
	scaleKeyDelta = vec2(0);//���Ű�������
}

float modelTranslateSensitivity = 0.002f;//ģ��ƽ�����ж�
float modelRotateSensitivity = 0.1f;//ģ����ת���ж�
float modelScaleSensitivity = 0.01f;//ģ���������ж�

