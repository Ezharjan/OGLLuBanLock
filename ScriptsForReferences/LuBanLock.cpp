#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Imgui/imgui.h"
#include "Imgui/imgui_impl_glfw.h"
#include "Imgui/imgui_impl_opengl3.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <shader_s.h>
#include <camera.h>
#include <iostream>
#include <vector>
using namespace std;
using namespace glm;
/*…………………………………………结构体…………………………………………*/
/*………………………………回调函数………………………………*/
//当窗口大小改变时，将调用该回调函数以调整视口大小
void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

//鼠标信息
vec2 lastmousePosition = vec2(0, 0);//上一帧鼠标位置
vec2 mousePositionDelta = vec2(0);//鼠标位置增量
double mouseScrollDelta = 0.0f;//鼠标滚轮增量
bool isLeftMouseButtonRepeat = false;//鼠标左键是否按住
bool isLeftMouseButtonRelease = true;//鼠标左键是否抬起
bool isRightMouseButtonRepeat = false;//鼠标右键是否按住
bool isRightMouseButtonRelease = true;//鼠标右键是否抬起
//光标位置回调函数
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
	mousePositionDelta = vec2(xpos - lastmousePosition.x, lastmousePosition.y - ypos);//取得鼠标位置差值
	lastmousePosition = vec2(xpos, ypos);//记录该帧的鼠标位置
}

unsigned int isTranslate = false;//是否在平移模型
unsigned int isRotate = false;//是否在旋转任意模型
//鼠标按键回调函数
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	switch (action)
	{
	case GLFW_PRESS://按下鼠标的一帧调用
		switch (button)
		{
		case GLFW_MOUSE_BUTTON_LEFT://鼠标左键
			isLeftMouseButtonRelease = !(isTranslate = isLeftMouseButtonRepeat = true);
			break;
		case GLFW_MOUSE_BUTTON_RIGHT://鼠标右键
			isRightMouseButtonRelease = !(isRotate = isRightMouseButtonRepeat = true);
			break;
		}
		break;
	case GLFW_RELEASE://抬起鼠标的一帧调用
		switch (button)
		{
		case GLFW_MOUSE_BUTTON_LEFT://鼠标左键
			isTranslate = isLeftMouseButtonRepeat = !(isLeftMouseButtonRelease = true);
			break;
		case GLFW_MOUSE_BUTTON_RIGHT://鼠标右键
			isRotate = isRightMouseButtonRepeat = !(isRightMouseButtonRelease = true);
			break;
		}
		break;
	}
}
unsigned int isScale = false;//是否在缩放任意模型
//鼠标滚轮回调函数
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	mouseScrollDelta = yoffset;
	isScale = (yoffset != 0);
}

//键盘信息
vec4 translateKeyDelta = vec4(0);//平移按键增量
vec4 rotateKeyDelta = vec4(0);//旋转按键增量
vec2 scaleKeyDelta = vec2(0);//缩放按键增量

//按键回调函数
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	glfwSetWindowShouldClose(window, key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE);//当按下后抬起Esc键则关闭窗口
	if (action == GLFW_REPEAT)//如果是按下某个按键
	{
		translateKeyDelta = vec4(key == GLFW_KEY_W, key == GLFW_KEY_S, key == GLFW_KEY_A, key == GLFW_KEY_D);//WASD控制移动
		rotateKeyDelta = vec4(key == GLFW_KEY_UP, key == GLFW_KEY_DOWN, key == GLFW_KEY_LEFT, key == GLFW_KEY_RIGHT);//上下左右控制旋转
		scaleKeyDelta = vec2(key == GLFW_KEY_LEFT_BRACKET, key == GLFW_KEY_RIGHT_BRACKET);//[]控制缩放

		//为键盘增量增加倍数
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
/*………………窗口结构体………………*/
struct Window
{
private:

	/*设置回调函数*/
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
	const char* windowTitle = "LuBanLock";//窗口名称
	float windowWidth = 1920.0f;//窗口宽度
	float windowHeight = 1080.0f;//窗口高度
	//初始化窗口
	Window()
	{
		if (window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, NULL, NULL))//创建一个窗口对象，参数分别为宽、高、窗口名称，后两个暂时忽略
		{
			glfwMakeContextCurrent(window);//将窗口上下文设置为当前线程的主上下文
			SetCallback(window);
		}
		else
		{
			cout << "窗口创建失败！";
			glfwTerminate();
		}
		lastmousePosition = vec2(0, 0);//初始化鼠标位置
	}
};

mat4 view, projection;//视图、投影矩阵
mat4 outlineModel;//描边模型矩阵
unsigned int diffuseMap;//漫反射贴图
unsigned int specularMap;//高光贴图
unsigned int backgroundMap;//背景贴图
vec3 translateSum;//与世界坐标系原点的位移差
/*………………模型结构体………………*/
struct Model
{
public:
	mat4 model;//模型矩阵

	vector<vector<float>> verticesData;//顶点数据
	vector<vector<int>> dataIndices;//数据索引
	vector<vec3> animationPoint;//动画结点

	/*包围盒结构体*/
	struct SurroundBox
	{
	private:
		vec3* initSurroundPoint = new vec3[8];//初始包围点
		vec2* surroundPoint = new vec2[8];//包围点
		vec2* extramePoint = new vec2[4];//极端点
		vec2* leftPoint = new vec2[4];//剩余点
		enum RangePointType { xMin, xMax, yMin, yMax };//范围点类型

		//点是否在直线下方
		bool IsPointBelowLine(vec2 point, vec2 lineLeftPoint, vec2 lineRightPoint)
		{
			float A = (lineRightPoint.y) - (lineLeftPoint.y);
			float B = (lineLeftPoint.x) - (lineRightPoint.x);
			float C = (lineRightPoint.x * lineLeftPoint.y) - (lineLeftPoint.x * lineRightPoint.y);
			return ((A * point.x + B * point.y + C) > 0);
		}
	public:
		//初始化包围点
		SurroundBox(vector<vec3> points)
		{
			for (int i = 0; i < 8; i++)
			{
				initSurroundPoint[i] = points[i];
			}
		}
		//更新包围点
		void UpdateSurroundPoint(mat4 model, mat4 view, mat4 projection, Window window)
		{
			for (int i = 0; i < 8; i++)
			{
				vec4 tempPoint = (projection * view * model) * vec4(initSurroundPoint[i], 1);
				if (tempPoint.w != 0)
				{
					tempPoint /= tempPoint.w;
				}
				surroundPoint[i] = vec2(tempPoint.x * (window.windowWidth / 2) + (window.windowWidth / 2), tempPoint.y * -(window.windowHeight / 2) + (window.windowHeight / 2));//得到包围点在屏幕上的坐标
			}
		}

		//获取极端点
		void GetExtramePoint()
		{
			//初始化极端点
			for (size_t i = 0; i < 4; i++)
			{
				extramePoint[i] = surroundPoint[0];
			}

			//通过比较计算极端点
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

		/*获取剩余点*/
		void GetLeftPoint()
		{
			vector<vec2> tempPoint;//用于初始化剩余点的临时点
			bool isSame;//包围点是否与极端点一致
			//记录包围点中除去极端点的点
			for (size_t i = 0; i < 8; i++)//遍历包围点
			{
				isSame = false;//假设是新的包围点
				for (size_t j = 0; j < 4; j++)//遍历极端点
				{
					if (surroundPoint[i] == extramePoint[j])//如果某个包围点和某个极端点一致
					{
						isSame = true;//设定包围点与极端点一致
						break;
					}
				}
				if (!isSame)//如果与四个极端点都不一样
				{
					tempPoint.push_back(surroundPoint[i]);//记录此点
				}
			}

			//初始化剩余点
			for (size_t i = 0; i < 4; i++)
			{
				leftPoint[i] = tempPoint[0];
			}

			//通过比较计算剩余点
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

		/*鼠标是否在包围盒上*/
		bool IsMouseSelect()
		{
			bool IsInExtramePointAera =
				!IsPointBelowLine(lastmousePosition, extramePoint[RangePointType::xMin], extramePoint[RangePointType::yMin])
				&& IsPointBelowLine(lastmousePosition, extramePoint[RangePointType::yMax], extramePoint[RangePointType::xMax])
				&& IsPointBelowLine(lastmousePosition, extramePoint[RangePointType::xMin], extramePoint[RangePointType::yMax])
				&& !IsPointBelowLine(lastmousePosition, extramePoint[RangePointType::yMin], extramePoint[RangePointType::xMax]);//是否在极端点构成的区域内

			bool IsInLeftPointAera =
				!IsPointBelowLine(lastmousePosition, leftPoint[RangePointType::xMin], leftPoint[RangePointType::yMin])
				&& IsPointBelowLine(lastmousePosition, leftPoint[RangePointType::yMax], leftPoint[RangePointType::xMax])
				&& IsPointBelowLine(lastmousePosition, leftPoint[RangePointType::xMin], leftPoint[RangePointType::yMax])
				&& !IsPointBelowLine(lastmousePosition, leftPoint[RangePointType::yMin], leftPoint[RangePointType::xMax]);//是否在剩余点构成的区域内

			return (IsInExtramePointAera || IsInLeftPointAera);//取区域的并集
		}
	};

	vector<SurroundBox> surroundBoxs;//包围盒

	//初始化顶点数据
	Model(vector<float> vertices_data, int trianglesNumber, vector<vector<vec3>> surroundPoints, vector<vec3>animationPoint)
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
			surroundBoxs.push_back(SurroundBox(surroundPoints[i]));//根据包围点生成包围盒
		}

		this->animationPoint.push_back(vec3(0));
		this->animationPoint.insert(this->animationPoint.end(), animationPoint.begin(), animationPoint.end());//初始化动画点
	}

	//初始化顶点数据
	Model(vector<float> vertices_data, int trianglesNumber)
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

	/*配置属性(顶点位置、纹理坐标等)*/
	void ConfigAttribute(unsigned int location, unsigned int count, GLenum type, bool normalized, unsigned int stride, const void* pointer)
	{
		glVertexAttribPointer(location, count, type, normalized, stride, pointer);
		glEnableVertexAttribArray(location);
	}
	/*解绑对象*/
	void UnBindObject()
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);//解绑VBO、EBO
		glBindVertexArray(0);//解绑VAO
	}

	/*设置变换属性*/
	void SetTransform(vec3 translateDelta, vec2 RotateAngle, vec3 scaleDelta)
	{
		//进行世界坐标系下缩放
		model = translate(mat4(1), -translateSum) * model;
		model = scale(mat4(1), scaleDelta) * model;
		model = translate(mat4(1), translateSum) * model;

		//进行世界坐标系下旋转
		model = translate(mat4(1), -translateSum) * model;
		model = rotate(mat4(1), radians(RotateAngle.x), vec3(0, 1, 0)) * rotate(mat4(1), radians(RotateAngle.y), vec3(-1, 0, 0)) * model;//将模型矩阵平移到世界坐标原点在旋转
		model = translate(mat4(1), translateSum) * model;

		//平移
		model = translate(mat4(1), translateDelta) * model;
	}

	//设置动画
	void SetAnimation(float animationTime)
	{
		int animationIndex = 0;
		//确定动画索引
		while (animationTime >= ((animationIndex) * (1.0f / (animationPoint.size() - 1))))//如果动画时间大于某个动画结点对应的时间
		{
			++animationIndex;//动画索引自增
		}
		vec3 targetPosition =
			(
			(
				(animationTime - ((animationIndex - 1) * (1.0f / (animationPoint.size() - 1)))) / (1.0f / (animationPoint.size() - 1))
				)
				* (animationPoint[animationIndex] - animationPoint[animationIndex - 1])
				)
			+ animationPoint[animationIndex - 1];//获取移动插值
		model = translate(model, targetPosition - position);//获取物体目标位置
		position = targetPosition;//记录目标位置
	}

	/*设置对单个物体进行聚焦的情况*/
	mat4 recordedModelMatrix;//记录的模型矩阵
	void SetFocus(bool isFocus)
	{
		if (isFocus)
		{
			recordedModelMatrix = model;//记录模型矩阵
			model = mat4(1);//重置模型矩阵
		}
		else
		{
			model = recordedModelMatrix;//复原模型矩阵
		}
	}

	/*设置模型、视图、投影矩阵*/
	void SetMVPMatrix(Shader shader)
	{
		shader.use();
		shader.setMat4("model", model);
		shader.setMat4("view", view);
		shader.setMat4("projection", projection);
	}

	/*使用索引，绘制对象*/
	void DrawObject(GLenum mode, unsigned int count, GLenum type, const void* indices)
	{
		glBindVertexArray(this->VAO);
		glDrawElements(mode, count, type, indices);
	}

	/*鼠标是否选中物体*/
	bool IsMouseSelect(Window window)
	{
		for (SurroundBox surroundBox : surroundBoxs)//遍历所有包围盒
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

	//重置变换
	void ResetTransform()
	{
		model = mat4(1);
		translateSum = vec3(0);
	}

	/*释放空间*/
	void ReleaseSpace()
	{
		glDeleteBuffers(1, &VBO);
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &EBO);
	}

private:
	float* vertices_data;//顶点数据
	int* indices_data;//索引数据
	enum class ObjectType { VAO, VBO, EBO };//对象类型
	unsigned int VBO;//顶点缓存对象
	unsigned int VAO;//顶点数组对象
	unsigned int EBO;//索引缓冲对象

	vec3 position = vec3(0);//模型位置
	/*初始化VBO与VAO*/
	void InitVBOandVAO()
	{
		/*将接收的顶点数据存储下来*/
		vertices_data = new float[verticesData[0].size()];
		for (size_t i = 0; i < verticesData[0].size(); i++)
		{
			vertices_data[i] = verticesData[0][i];
		}

		glGenBuffers(1, &VBO);//生成VBO
		BindObject(ObjectType::VBO);
		glBufferData(GL_ARRAY_BUFFER, verticesData[0].size() * sizeof(float), vertices_data, GL_STATIC_DRAW);//配置顶点数据
		glGenVertexArrays(1, &VAO);//生成VAO
		BindObject(ObjectType::VAO);
	}
	/*初始化EBO*/
	void InitEBO()
	{
		/*将接收的索引数据存储下来*/
		indices_data = new int[dataIndices[0].size()];
		for (size_t i = 0; i < dataIndices[0].size(); i++)
		{
			indices_data[i] = dataIndices[0][i];
		}
		glGenBuffers(1, &EBO);//生成EBO
		BindObject(ObjectType::EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, dataIndices[0].size() * sizeof(int), indices_data, GL_STATIC_DRAW);//配置索引数据
	}
	/*绑定对象*/
	void BindObject(ObjectType objectType)
	{
		switch (objectType)
		{
		case Model::ObjectType::VAO:
			glBindVertexArray(VAO);//绑定VAO
			break;
		case Model::ObjectType::VBO:
			glBindBuffer(GL_ARRAY_BUFFER, VBO);//绑定VBO
			break;
		case Model::ObjectType::EBO:
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);//绑定EBO
			break;
		}
	}
};
/*…………………………………………全局方法…………………………………………*/
/*………………………………设置矩阵………………………………*/
//设置视图矩阵
void SetViewMatrix()
{
	view = translate(view, vec3(0, 0, -3));
}
//设置投影矩阵
void SetProjectionMatrix(Window window, bool isOrtho, float fov = 45.0f)
{
	projection = (isOrtho) ? ortho(-window.windowWidth / 1000, window.windowWidth / 1000, -window.windowHeight / 1000, window.windowHeight / 1000, 0.1f, 100.0f) : perspective(radians(fov), window.windowWidth / window.windowHeight, 0.1f, 100.0f);//设定投影视图是正交或透视
}
/*………………………………时间………………………………*/
float deltaTime = 0;//每两帧的时间差
//获取每两帧之间时间差
void GetDeltaTime()
{
	float lastFrame = 0.0f;
	float currentTime = 0.0f;
	currentTime = glfwGetTime();//1.记录当前帧对应时间
	deltaTime = currentTime - lastFrame;//3.从第二帧开始，获得与前帧的时间差
	lastFrame = currentTime;//2.缓存当前帧时间，作为下一帧的前帧时间
}
/*………………………………初始化………………………………*/
//初始化GLFW
void InitGLFW()
{
	glfwInit();//初始化GLFW
	/*glfwWindowHint用于配置GLFW，第一个参数是选项名称，第二个是设置选项的一个整形的值*/
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//设定主版本号为3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//设定次版本号为3
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//告诉GLFW使用核心模式
}
//初始化GLAD
void InitGLAD()
{
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))//glfwGetProcAddress是用于加载系统相关的OpenGL函数指针地址的函数(必须在设定好线程上下文之后调用)
	{
		cout << "初始化GLAD失败" << endl;
		glfwTerminate();
	}
	gladLoadGL();
}
//初始化Imgui
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

vector<Model*> modelsbyAnimationSequence;//按照动画顺序排列的模型数组
//初始化动画顺序
void InitAnimationSequence(vector<Model*> _models, vector<int> animation_sequence)
{
	for (size_t i = 0; i < _models.size(); i++)
	{
		modelsbyAnimationSequence.push_back(_models[animation_sequence[i]]);//根据顺序记录所有模型
	}
}
vector<Model*>models;//模型数组
//初始化模型数组
void InitModels(vector<Model*>_models)
{
	for (Model* model : _models)
	{
		models.push_back(model);
	}
}
vec3 translateSumforRecord;//用于记录的平移之和
vector<mat4> modelsforRecord;//用于记录的模型矩阵数组
//初始化记录矩阵
void InitRecordedModels()
{
	for (Model* model : models)
	{
		modelsforRecord.push_back(model->model);
	}
}
/*生成纹理*/
unsigned int GenTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);//生成材质

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);//加载图片
	if (data)//如果加载图片成功
	{
		GLenum format = (nrComponents == 3) ? GL_RGB : ((nrComponents == 4) ? GL_RGBA : GL_RED);//根据图片信息设定色彩模式

		glBindTexture(GL_TEXTURE_2D, textureID);//绑定图片到纹理
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);//生成纹理
		glGenerateMipmap(GL_TEXTURE_2D);//生成多级渐远纹理

		/*设置环绕与过滤模式*/
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);//释放图片信息
	}
	else
	{
		cout << "纹理无法从以下路径加载：" << path << endl;
		stbi_image_free(data);
	}

	return textureID;
}
/*激活与绑定模型纹理*/
void ActiveandBindModelsTexture()
{
	glActiveTexture(GL_TEXTURE0);//激活漫反射纹理
	glBindTexture(GL_TEXTURE_2D, diffuseMap);//绑定漫反射纹理
	glActiveTexture(GL_TEXTURE1);//激活高光纹理
	glBindTexture(GL_TEXTURE_2D, specularMap);//绑定高光纹理
}
/*激活与绑定背景纹理*/
void ActiveandBindBackgroundTexture()
{
	glActiveTexture(GL_TEXTURE0);//激活漫反射纹理
	glBindTexture(GL_TEXTURE_2D, backgroundMap);//绑定漫反射纹理
}
//初始化模型贴图
void InitModelTexture(Shader modelShader, char const* diffuseMapPath, char const* specularMapPath)
{
	stbi_set_flip_vertically_on_load(true);//设定加载图片时反转y轴
	diffuseMap = GenTexture(diffuseMapPath);//根据路径生成漫反射纹理贴图
	specularMap = GenTexture(specularMapPath);//根据路径生成高光纹理贴图
	modelShader.use();
	modelShader.setInt("material.diffuse", 0);
	modelShader.setInt("material.specular", 1);
}
//初始化模型贴图
void InitModelTexture(Shader backgroundShader, char const* mapPath)
{
	stbi_set_flip_vertically_on_load(true);//设定加载图片时反转y轴
	backgroundMap = GenTexture(mapPath);//根据路径生成贴图
	backgroundShader.use();
	backgroundShader.setInt("backgroundTexture", 0);
}
/*………………………………灯光………………………………*/
vec3 lightPosition;//灯光位置
//生成点光源
void GenSpotLight(Shader modelShader, vec3 position, vec3 direction, float innerCutOffAngle, float outerCutOffAngle)
{
	modelShader.use();
	modelShader.setVec3("light.position", position);
	modelShader.setVec3("light.direction", direction);
	modelShader.setFloat("light.innerCutOff", cos(radians(innerCutOffAngle)));//将角度的余弦值传入
	modelShader.setFloat("light.outerCutOff", cos(radians(outerCutOffAngle)));
	modelShader.setVec3("viewPosition", position);
}
//设置光源属性
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
/*………………………………动画………………………………*/
//根据给出的动画时间点播放动画
void PlayAnimation(float animationTime, Shader modelShader)
{
	if (animationTime < 1)//确保时间不会超过1
	{
		int animationSequenceIndex = 0;//动画顺序索引
		for (int i = 0; i < modelsbyAnimationSequence.size(); i++)
		{
			//获取动画顺序索引
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
		animationTime -= (animationSequenceIndex) * (1.0f / modelsbyAnimationSequence.size());//将时间确定到单个物体对应的动画时间
		modelsbyAnimationSequence[animationSequenceIndex]->SetAnimation(animationTime / (1.0f / modelsbyAnimationSequence.size()));//播放对应物体的动画
	}
}
/*………………………………测试设置………………………………*/
//设置测试是否启用
void SetTestEnable(GLenum test, bool isEnable)
{
	if (isEnable)
	{
		glEnable(test);//开启测试
	}
	else
	{
		glDisable(test);//禁用测试
	}
}
//设置模板写入
void SetStencilWrite(bool isWritable)
{
	glStencilMask((isWritable) ? 0xFF : 0x00);//设置模板掩码来控制对模板缓冲的写入
}
/*………………………………模型………………………………*/
//配置模型
void ConfigModel(Model model)
{
	model.ConfigAttribute(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	model.ConfigAttribute(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	model.ConfigAttribute(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	model.UnBindObject();
}
//设置所有模型变换
void SetAllModelsTransform(vec3 translateDelta, vec2 RotateAngle, vec3 scaleDelta)
{
	for (Model* model : models)
	{
		model->SetTransform(translateDelta, RotateAngle, scaleDelta);
	}
}

//绘制描边
void DrawOutline(Model* model, Window mainWindow, Shader outlineShader, int trianglesNumber)
{
	outlineShader.use();
	outlineModel = model->model;
	outlineModel = scale(outlineModel, vec3(1.05f, 1.05f, 1.05f));
	outlineShader.setMat4("model", outlineModel);
	outlineShader.setMat4("view", view);
	outlineShader.setMat4("projection", projection);
	model->DrawObject(GL_TRIANGLES, trianglesNumber * 3, GL_UNSIGNED_INT, 0);
}

int outlineIndex = -1;//需要描边物体的索引
bool isAnyModelFocused = false;//是否有物体被聚焦
int focusedModelIndex = -1;//被聚焦的物体索引
bool isRecordedModelMartix = false;//是否记录了模型矩阵
//绘制模型
void DrawModels(vector<Model*> models, vector<int> trianglesNumber, Window mainWindow, Shader modelShader, Shader outlineShader)
{
	if (!isAnyModelFocused)//如果没有任何模型被聚焦
	{
		outlineIndex = -1;//假定没有物体需要被描边

		for (size_t i = 0; i < (models.size()); i++)
		{
			models[i]->SetMVPMatrix(modelShader);

			if (!ImGui::IsAnyItemActive() && !ImGui::IsAnyWindowHovered() && models[i]->IsMouseSelect(mainWindow))//除去鼠标在ImGui上的情况
			{
				{
					SetTestEnable(GL_STENCIL_TEST, true);
					SetStencilWrite(true);
					glStencilFunc(GL_ALWAYS, 1, 0xFF);//不受模板测试的影响下渲染物体
				}
				models[i]->DrawObject(GL_TRIANGLES, (3 * trianglesNumber[i]), GL_UNSIGNED_INT, 0);
				outlineIndex = i;//记录需要被描边的物体索引值
				if (ImGui::IsMouseClicked(0))//如果按下鼠标左键
				{
					focusedModelIndex = i;//记录该物体的索引
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

		if (outlineIndex != -1)//如果有物体需要被描边
		{
			{
				SetTestEnable(GL_STENCIL_TEST, true);
				glStencilFunc(GL_NOTEQUAL, 1, 0xFF);//当模板值不为1才渲染
				SetTestEnable(GL_DEPTH_TEST, false);
			}
			DrawOutline(models[outlineIndex], mainWindow, outlineShader, trianglesNumber[outlineIndex]);
			{
				SetStencilWrite(true);
				glClear(GL_STENCIL_BUFFER_BIT);//清空模板值
				SetStencilWrite(false);
			}
		}
	}
	else//如果有任何模型被聚焦
	{
		bool isNeedOutline = false;//假定物体不需要被描边

		models[focusedModelIndex]->SetMVPMatrix(modelShader);
		if (!ImGui::IsAnyItemActive() && !ImGui::IsAnyWindowHovered() && models[focusedModelIndex]->IsMouseSelect(mainWindow))
		{
			{
				SetTestEnable(GL_STENCIL_TEST, true);
				SetStencilWrite(true);
				glStencilFunc(GL_ALWAYS, 1, 0xFF);//不受模板测试的影响下渲染物体
			}
			models[focusedModelIndex]->DrawObject(GL_TRIANGLES, (3 * trianglesNumber[focusedModelIndex]), GL_UNSIGNED_INT, 0);
			isNeedOutline = true;//物体需要被描边
			if (ImGui::IsMouseClicked(0))//如果按下鼠标左键
			{
				focusedModelIndex = -1;//设定该物体不再被聚焦
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

		if (isNeedOutline)//如果物体需要被描边
		{
			{
				SetTestEnable(GL_STENCIL_TEST, true);
				glStencilFunc(GL_NOTEQUAL, 1, 0xFF);//当模板值不为1才渲染
				SetTestEnable(GL_DEPTH_TEST, false);
			}
			DrawOutline(models[focusedModelIndex], mainWindow, outlineShader, trianglesNumber[focusedModelIndex]);
			{
				SetStencilWrite(true);
				glClear(GL_STENCIL_BUFFER_BIT);//清空模板值
				SetStencilWrite(false);
			}
		}
	}
}

//设置卡通渲染
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

//渲染背景
void RenderBackground(Shader backGroundShader, Model background)
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
/*………………………………渲染……………………………………*/
//清除所有缓冲
void ClearAllBuffer()
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);//设置窗口填充颜色
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);//清除颜色、深度与模板缓冲
}
/*…………………………………………ImGUI设置…………………………………………*/
//设置Imgui新帧
void SetImguiNewFrame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

int isOrtho = false;//是否为正交投影
float fov = 45.0f;//透视角度
static char backgroundPath[64] = "Resources/Textures/Background/02.png";//背景图片路径
static const char* modelMaps[9][2] = //模型贴图
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
static int modelMapIndex = 0;//模型贴图索引
float lightDepth = 3;//灯光深度
float innerCutoffAngle = 12.5f;//内光切
float outerCutoffAngle = 17.5f;//外光切
ImVec4 ambient(0.1, 0.1, 0.1, 1);//环境光
ImVec4 diffuse(0.8f, 0.8f, 0.8f, 1.0f);//漫反射光
ImVec4 specular(1.0f, 1.0f, 1.0f, 1.0f);//高光
int shininess = 2;//光泽度
bool isLockLightPoiiton = false;//是否锁定灯光位置
bool isAnimationPlaying = false;//动画是否在播放
float animationTime = 0.0f;//总动画时间结点
bool isUseNPR = false;//是否使用NPR
float NPRColorWeight[4] = { 0.75f,0.5f,0.25f,0.0f };//NPC颜色权重
ImVec4 NPRColor[4] = { ImVec4(1,0.5,0.5,1),ImVec4(0.6,0.3,0.3,1), ImVec4(0.4,0.2,0.2,1), ImVec4(0.2,0.1,0.1,1) };//NPR颜色
//生成Imgui主窗口
void GenImguiMainWindow(Shader modelShader, Shader backGroundShader)
{
	ImGui::Begin(u8"欢迎使用鲁班锁教学演示程序！", NULL, ImGuiWindowFlags_NoMove);
	ImGui::SetWindowPos(ImVec2(0, 0));
	ImGui::Spacing();
	if (ImGui::CollapsingHeader(u8"简介"))
	{
		ImGui::Indent();
		ImGui::Bullet(); ImGui::TextWrapped(u8"欢迎，您可以在此程序中体会到鲁班锁的魅力所在！");
		ImGui::Bullet(); ImGui::TextWrapped(u8"您可以通过鼠标和键盘对模型进行交互，使用此UI面板来进行个性化配置");
		ImGui::Unindent();
	}
	ImGui::Spacing();
	if (ImGui::CollapsingHeader(u8"交互说明"))
	{
		{
			ImGui::Indent();
			if (ImGui::TreeNode(u8"非UI界面"))
			{
				ImGui::Indent();
				if (ImGui::TreeNode(u8"鼠标"))
				{
					ImGui::Indent();
					ImGui::Bullet(); ImGui::TextWrapped(u8"当鼠标位于非图形界面空白区域时，您可以使用其来与物体进行交互");

					ImGui::Indent();
					if (ImGui::TreeNode(u8"平移"))
					{
						ImGui::Bullet(); ImGui::TextWrapped(u8"拖动鼠标左键来平移物体");
						ImGui::TreePop();
					}
					if (ImGui::TreeNode(u8"旋转"))
					{
						ImGui::Bullet(); ImGui::TextWrapped(u8"拖动鼠标右键来平移物体");
						ImGui::TreePop();
					}
					if (ImGui::TreeNode(u8"缩放"))
					{
						ImGui::Bullet(); ImGui::TextWrapped(u8"使用鼠标滚轮来缩放物体");
						ImGui::TreePop();
					}
					if (ImGui::TreeNode(u8"聚焦模式"))
					{
						ImGui::Bullet(); ImGui::TextWrapped(u8"鼠标放置在模型上会有描边提示");
						ImGui::Bullet(); ImGui::TextWrapped(u8"此时单击鼠标左键可以进入聚焦模式，单独查看选中的物体");
						ImGui::TreePop();
					}
					ImGui::Unindent();
					ImGui::Unindent();
					ImGui::TreePop();
				}
				if (ImGui::TreeNode(u8"键盘"))
				{
					ImGui::Indent();
					ImGui::Bullet(); ImGui::TextWrapped(u8"使用键盘交互时，请确保处于英文输入状态");

					ImGui::Indent();
					if (ImGui::TreeNode(u8"平移"))
					{
						ImGui::Bullet(); ImGui::TextWrapped(u8"使用WASD键来平移物体");
						ImGui::TreePop();
					}
					if (ImGui::TreeNode(u8"旋转"))
					{
						ImGui::Bullet(); ImGui::TextWrapped(u8"使用方向键来旋转物体");
						ImGui::TreePop();
					}
					if (ImGui::TreeNode(u8"缩放"))
					{
						ImGui::Bullet(); ImGui::TextWrapped(u8"使用\"[ ]\"键来缩放物体");
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
			if (ImGui::TreeNode(u8"UI界面"))
			{
				ImGui::Indent();
				ImGui::Bullet(); ImGui::TextWrapped(u8"使用鼠标进行点击、拖动操作；键盘进行输入操作");
				ImGui::Bullet(); ImGui::TextWrapped(u8"按住Ctrl键点击滑动条可以使用键盘输入数值");
				ImGui::Unindent();
				ImGui::TreePop();
			}
			ImGui::Unindent();
		}
	}
	ImGui::Spacing();
	if (ImGui::CollapsingHeader(u8"投影设置"))
	{
		ImGui::Indent();
		ImGui::RadioButton(u8"透视投影", &isOrtho, 0);
		if (!isOrtho)
		{
			ImGui::SameLine(); ImGui::SliderFloat(u8"视场(FOV)", &fov, 0, 45);
		}
		ImGui::RadioButton(u8"正交投影", &isOrtho, 1);
		ImGui::Unindent();
	}
	ImGui::Spacing();
	if (ImGui::CollapsingHeader(u8"背景设置"))
	{
		ImGui::Indent();
		if (ImGui::InputTextWithHint(u8"背景图片路径", u8"注意不可使用中文路径且需要去掉""，按下回车键以确认", backgroundPath, 64, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			InitModelTexture(backGroundShader, backgroundPath);
		}
		ImGui::Unindent();
	}
	ImGui::Spacing();
	if (ImGui::CollapsingHeader(u8"模型设置"))
	{
		ImGui::Indent();
		if (ImGui::Combo(u8"选择模型贴图", &modelMapIndex, " 01\0 02\0 03\0 04\0 05\0 06\0 07\0 08\0 09\0"))
		{
			InitModelTexture(modelShader, modelMaps[modelMapIndex][0], modelMaps[modelMapIndex][1]);
		}
		ImGui::Checkbox(u8"是否开启卡通渲染", &isUseNPR);
		modelShader.use();
		modelShader.setBool("npr.isUseNPR", isUseNPR);
		if (isUseNPR)
		{
			ImGui::SliderFloat(u8"区域0", &NPRColorWeight[0], NPRColorWeight[1], 1);
			ImGui::ColorEdit4(u8"纯色0", (float*)&NPRColor[0], ImGuiColorEditFlags_Float);
			ImGui::Separator();
			ImGui::SliderFloat(u8"区域1", &NPRColorWeight[1], NPRColorWeight[2], NPRColorWeight[0]);
			ImGui::ColorEdit4(u8"纯色1", (float*)&NPRColor[1], ImGuiColorEditFlags_Float);
			ImGui::Separator();
			ImGui::SliderFloat(u8"区域2", &NPRColorWeight[2], NPRColorWeight[3], NPRColorWeight[1]);
			ImGui::ColorEdit4(u8"纯色2", (float*)&NPRColor[2], ImGuiColorEditFlags_Float);
			ImGui::Separator();
			ImGui::SliderFloat(u8"区域3", &NPRColorWeight[3], 0, NPRColorWeight[2]);
			ImGui::ColorEdit4(u8"纯色3", (float*)&NPRColor[3], ImGuiColorEditFlags_Float);
		}
		ImGui::Unindent();
	}
	ImGui::Spacing();
	if (ImGui::CollapsingHeader(u8"灯光设置"))
	{
		ImGui::Indent();
		ImGui::SliderFloat(u8"灯光深度", &lightDepth, 0, 10);
		if (!isUseNPR)
		{
			ImGui::SliderFloat(u8"内光切", &innerCutoffAngle, 0, outerCutoffAngle);
			ImGui::SliderFloat(u8"外光切", &outerCutoffAngle, innerCutoffAngle, 89);
			ImGui::Separator();
			ImGui::ColorEdit4(u8"环境光", (float*)&ambient, ImGuiColorEditFlags_Float);
			ImGui::ColorEdit4(u8"漫反射光", (float*)&diffuse, ImGuiColorEditFlags_Float);
			ImGui::ColorEdit4(u8"镜面光", (float*)&specular, ImGuiColorEditFlags_Float);
			ImGui::Text("2^"); ImGui::SameLine(); ImGui::SliderInt(u8"光泽度", &shininess, 0, 10);
			ImGui::Separator();
		}
		ImGui::Checkbox(u8"锁定灯光位置到屏幕中央", &isLockLightPoiiton);
		if (isLockLightPoiiton)
		{
			lightPosition = vec3(0, 0, lightDepth);
		}
		ImGui::Unindent();
	}
	ImGui::Spacing();
	if (ImGui::CollapsingHeader(u8"动画设置"))
	{
		ImGui::Indent();
		if (ImGui::Button(u8"重置"))
		{
			animationTime = 0;
		}
		ImGui::SameLine(); isAnimationPlaying = (!isAnimationPlaying) ? ImGui::Button(u8"播放") : !(ImGui::Button(u8"暂停"));
		if (isAnimationPlaying && animationTime < 1)
		{
			animationTime += (deltaTime / 50000);
		}
		ImGui::SameLine(); float multipliedAnimationTime = animationTime * 100; ImGui::SliderFloat(u8"动画进度条", &multipliedAnimationTime, 0, 100, "%.3f %%"); animationTime = multipliedAnimationTime / 100;
		ImGui::Unindent();
	}
	ImGui::End();
}
//渲染Imgui
void RenderImgui()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
/*…………………………………………重置…………………………………………*/
//重置鼠标增量
void ResetMouseDelta()
{
	mouseScrollDelta = 0.0f;
	mousePositionDelta = vec2(0);
}
//重置键盘增量
void ResetKeyDelta()
{
	translateKeyDelta = vec4(0);//平移按键增量
	rotateKeyDelta = vec4(0);//旋转按键增量
	scaleKeyDelta = vec2(0);//缩放按键增量
}

float modelTranslateSensitivity = 0.002f;//模型平移敏感度
float modelRotateSensitivity = 0.1f;//模型旋转敏感度
float modelScaleSensitivity = 0.01f;//模型缩放敏感度
/*…………………………………………主函数…………………………………………*/
int main()
{
	/*………………………………初始化………………………………*/
	InitGLFW();
	Window mainWindow;
	InitGLAD();
	InitImgui(mainWindow);

	Shader modelShader("Model.vert", "Model.frag");//模型着色器
	Shader backGroundShader("Background.vert", "Background.frag");//背景着色器
	Shader outlineShader("Outline.vert", "Outline.frag");//描边着色器

	SetViewMatrix();

	//开启混合
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	//初始化模型
	Model model1
	(
		{
			-0.001,0.6544,0.1297,0.7179,0.8387,-0.0008,1,0.0006,
0.1278,0.6536,0.258,0.7077,0.7943,0.5789,0.5752,0.578,
0.1275,0.6543,0.1295,0.7349,0.8114,0.706,0.7082,0.0003,

0.1278,-0.6533,0.258,0.4516,0.856,0.5785,-0.5757,0.5778,
-0.001,-0.6541,0.1297,0.4197,0.8236,-0.0004,-1,-0.0004,
0.1277,-0.6536,0.1295,0.4519,0.8239,0.7085,-0.7057,-0.0006,

0.1279,-0.0009,0.258,0.9354,0.861,0.7081,0.0001,0.7061,
0.1276,-0.0009,0.1301,0.9184,0.8882,0.708,0.0004,-0.7062,
0.1279,0.2634,0.258,0.8794,0.8258,0.7077,-0.0001,0.7065,

-0.1296,-0.0009,0.2581,0.9697,0.8065,-0.7072,0,0.707,
-0.0008,0.2634,0.2582,0.8966,0.7986,0.0003,0.0001,1,
-0.1296,0.2634,0.2581,0.9137,0.7712,-0.7072,0.0001,0.707,

-0.1296,0.2637,0.1299,0.9307,0.744,-0.9042,-0.301,-0.3029,
-0.1296,-0.0007,0.13,0.9868,0.7793,-0.8162,0.4066,-0.4105,
-0.1296,0.2634,0.2581,0.9137,0.7712,-0.7072,0.0001,0.707,

-0.1296,-0.0007,0.13,0.9868,0.7793,-0.8162,0.4066,-0.4105,
-0.1296,-0.0009,0.2581,0.9697,0.8065,-0.7072,0,0.707,
-0.1296,0.2634,0.2581,0.9137,0.7712,-0.7072,0.0001,0.707,

0.1276,-0.0009,0.1301,0.9286,0.9326,0.708,0.0004,-0.7062,
-0.0009,-0.0007,0.1296,0.9114,0.9597,0.3002,0.3005,-0.9053,
-0.001,0.2634,0.1297,0.8554,0.9246,0.0003,-0.7048,-0.7094,

0.1278,0.2643,0.1298,0.8724,0.8972,0.905,-0.3006,-0.3009,
0.1276,-0.0009,0.1301,0.9286,0.9326,0.708,0.0004,-0.7062,
-0.001,0.2634,0.1297,0.8554,0.9246,0.0003,-0.7048,-0.7094,

0.1277,-0.6536,0.1295,0.4844,0.792,0.7085,-0.7057,-0.0006,
0.1277,-0.2633,0.1298,0.4853,0.6942,0.9046,0.3005,-0.3024,
0.1278,-0.6533,0.258,0.5166,0.7922,0.5785,-0.5757,0.5778,

0.1279,-0.2629,0.258,0.5174,0.6944,0.7079,-0.0001,0.7063,
0.1278,-0.6533,0.258,0.5166,0.7922,0.5785,-0.5757,0.5778,
0.1277,-0.2633,0.1298,0.4853,0.6942,0.9046,0.3005,-0.3024,

0.1278,0.6536,0.258,0.7967,0.7739,0.5789,0.5752,0.578,
0.1279,0.2634,0.258,0.8794,0.8258,0.7077,-0.0001,0.7065,
0.1278,0.2643,0.1298,0.8621,0.8529,0.905,-0.3006,-0.3009,

-0.1296,-0.6535,0.2581,0.3872,0.8554,-0.5779,-0.5763,0.5779,
-0.0008,-0.6536,0.2581,0.4194,0.8557,0.0006,-0.7059,0.7083,
-0.1296,-0.2629,0.2581,0.3864,0.9532,-0.7072,-0.0001,0.707,

-0.0008,0.6539,0.2581,0.8138,0.7465,0.0006,0.7064,0.7078,
-0.0008,0.2634,0.2582,0.8966,0.7986,0.0003,0.0001,1,
0.1279,0.2634,0.258,0.8794,0.8258,0.7077,-0.0001,0.7065,

-0.1296,-0.6536,0.1298,0.3556,0.7908,-0.45,-0.893,-0.0018,
-0.1296,-0.6535,0.2581,0.3234,0.7905,-0.5779,-0.5763,0.5779,
-0.1296,-0.2629,0.1298,0.3564,0.693,-1,0,0.0001,

-0.1296,0.6539,0.1298,0.848,0.692,-0.7081,0.7061,0.0004,
-0.1296,0.2637,0.1299,0.9307,0.744,-0.9042,-0.301,-0.3029,
-0.1296,0.6537,0.2581,0.831,0.7192,-0.578,0.576,0.578,

-0.1296,0.2634,0.2581,0.9137,0.7712,-0.7072,0.0001,0.707,
-0.1296,0.6537,0.2581,0.831,0.7192,-0.578,0.576,0.578,
-0.1296,0.2637,0.1299,0.9307,0.744,-0.9042,-0.301,-0.3029,

-0.0014,-0.2636,0.0009,0.4208,0.6937,0.2994,0.3001,-0.9057,
0.1274,-0.2634,0.0009,0.4531,0.694,0.5766,0.577,-0.5784,
0.1274,-0.6535,0.0009,0.4522,0.7916,0.5772,-0.5763,-0.5785,

0.1274,-0.6535,0.0009,0.4522,0.7916,0.5772,-0.5763,-0.5785,
-0.0011,-0.6536,0.0009,0.42,0.7914,-0.0015,-0.7063,-0.708,
-0.0014,-0.2636,0.0009,0.4208,0.6937,0.2994,0.3001,-0.9057,

-0.0011,0.6539,0.0009,0.7453,0.8558,-0.0007,0.7058,-0.7085,
-0.0011,0.2642,0.0009,0.828,0.9075,-0.0008,-0.7053,-0.7089,
-0.1296,0.2646,0.0009,0.8109,0.9347,-0.5785,-0.5744,-0.5791,

-0.001,-0.6541,0.1297,0.4197,0.8236,-0.0004,-1,-0.0004,
0.1274,-0.6535,0.0009,0.4522,0.7916,0.5772,-0.5763,-0.5785,
0.1277,-0.6536,0.1295,0.4519,0.8239,0.7085,-0.7057,-0.0006,

0.1277,-0.2633,0.1298,0.4853,0.6942,0.9046,0.3005,-0.3024,
0.1277,-0.6536,0.1295,0.4844,0.792,0.7085,-0.7057,-0.0006,
0.1274,-0.6535,0.0009,0.4522,0.7916,0.5772,-0.5763,-0.5785,

-0.001,0.6544,0.1297,0.7179,0.8387,-0.0008,1,0.0006,
-0.1296,0.6539,0.0009,0.7282,0.8831,-0.5779,0.5765,-0.5777,
-0.1296,0.6539,0.1298,0.7009,0.866,-0.7081,0.7061,0.0004,

0.1275,0.6543,0.1295,0.7794,0.8011,0.706,0.7082,0.0003,
0.1278,0.2643,0.1298,0.8621,0.8529,0.905,-0.3006,-0.3009,
0.1274,0.2643,0.0009,0.845,0.8802,0.5776,-0.5758,-0.5787,

-0.001,0.2634,0.1297,0.8554,0.9246,0.0003,-0.7048,-0.7094,
0.1274,0.2643,0.0009,0.845,0.8802,0.5776,-0.5758,-0.5787,
0.1278,0.2643,0.1298,0.8724,0.8972,0.905,-0.3006,-0.3009,

-0.1296,0.6539,0.1298,0.848,0.692,-0.7081,0.7061,0.0004,
-0.1296,0.6539,0.0009,0.8652,0.6647,-0.5779,0.5765,-0.5777,
-0.1296,0.2637,0.1299,0.9307,0.744,-0.9042,-0.301,-0.3029,

0.1279,-0.2629,0.258,0.0948,0.8366,0.7079,-0.0001,0.7063,
0.1277,-0.2633,0.1298,0.0723,0.8595,0.9046,0.3005,-0.3024,
0.1279,-0.0009,0.258,0.0481,0.7905,0.7081,0.0001,0.7061,

-0.1296,-0.2629,0.2581,0.1401,0.7906,-0.7072,-0.0001,0.707,
-0.0008,-0.2629,0.2582,0.1175,0.8136,0.0003,-0.0001,1,
-0.0008,-0.0009,0.2582,0.0708,0.7675,0.0003,0,1,

-0.1296,-0.2629,0.2581,0.1401,0.7906,-0.7072,-0.0001,0.707,
-0.1296,-0.0009,0.2581,0.0934,0.7446,-0.7072,0,0.707,
-0.1296,-0.0007,0.13,0.1159,0.7217,-0.8162,0.4066,-0.4105,

-0.001,-0.2629,0.1297,0.0496,0.8823,0.5772,0.5757,-0.5791,
-0.0009,-0.0007,0.1296,0.003,0.8361,0.3002,0.3005,-0.9053,
0.1277,-0.2633,0.1298,0.0723,0.8595,0.9046,0.3005,-0.3024,

0.1276,-0.0009,0.1301,0.0256,0.8133,0.708,0.0004,-0.7062,
0.1277,-0.2633,0.1298,0.0723,0.8595,0.9046,0.3005,-0.3024,
-0.0009,-0.0007,0.1296,0.003,0.8361,0.3002,0.3005,-0.9053,

-0.1296,0.6537,0.2581,0.6736,0.849,-0.578,0.576,0.578,
-0.0008,0.6539,0.2581,0.6906,0.8216,0.0006,0.7064,0.7078,
-0.001,0.6544,0.1297,0.7179,0.8387,-0.0008,1,0.0006,

-0.1296,-0.6535,0.2581,0.3872,0.8554,-0.5779,-0.5763,0.5779,
-0.001,-0.6541,0.1297,0.4197,0.8236,-0.0004,-1,-0.0004,
-0.0008,-0.6536,0.2581,0.4194,0.8557,0.0006,-0.7059,0.7083,

0.1279,-0.0009,0.258,0.9354,0.861,0.7081,0.0001,0.7061,
-0.0008,0.2634,0.2582,0.8966,0.7986,0.0003,0.0001,1,
-0.0008,-0.0009,0.2582,0.9526,0.8338,0.0003,0,1,

-0.0009,-0.0007,0.1296,0.9114,0.9597,0.3002,0.3005,-0.9053,
-0.1296,-0.0007,0.13,0.8943,0.987,-0.8162,0.4066,-0.4105,
-0.001,0.2634,0.1297,0.8554,0.9246,0.0003,-0.7048,-0.7094,

-0.1296,-0.0007,0.13,0.8943,0.987,-0.8162,0.4066,-0.4105,
-0.1296,0.2637,0.1299,0.8382,0.9519,-0.9042,-0.301,-0.3029,
-0.001,0.2634,0.1297,0.8554,0.9246,0.0003,-0.7048,-0.7094,

0.1278,-0.6533,0.258,0.4516,0.856,0.5785,-0.5757,0.5778,
0.1279,-0.2629,0.258,0.4509,0.9538,0.7079,-0.0001,0.7063,
-0.0008,-0.6536,0.2581,0.4194,0.8557,0.0006,-0.7059,0.7083,

-0.1296,0.6537,0.2581,0.831,0.7192,-0.578,0.576,0.578,
-0.1296,0.2634,0.2581,0.9137,0.7712,-0.7072,0.0001,0.707,
-0.0008,0.6539,0.2581,0.8138,0.7465,0.0006,0.7064,0.7078,

-0.1296,-0.2629,0.0009,0.3887,0.6933,-0.0001,0.0001,-1,
-0.0014,-0.2636,0.0009,0.4208,0.6937,0.2994,0.3001,-0.9057,
-0.1296,-0.653,0.0009,0.3878,0.791,-0.0032,-0.7045,-0.7097,

-0.0011,-0.6536,0.0009,0.42,0.7914,-0.0015,-0.7063,-0.708,
-0.1296,-0.653,0.0009,0.3878,0.791,-0.0032,-0.7045,-0.7097,
-0.0014,-0.2636,0.0009,0.4208,0.6937,0.2994,0.3001,-0.9057,

0.1275,0.6541,0.0009,0.7623,0.8284,0.5775,0.5768,-0.5777,
0.1274,0.2643,0.0009,0.845,0.8802,0.5776,-0.5758,-0.5787,
-0.0011,0.6539,0.0009,0.7453,0.8558,-0.0007,0.7058,-0.7085,

-0.1296,-0.653,0.0009,0.3878,0.791,-0.0032,-0.7045,-0.7097,
-0.0011,-0.6536,0.0009,0.42,0.7914,-0.0015,-0.7063,-0.708,
-0.001,-0.6541,0.1297,0.4197,0.8236,-0.0004,-1,-0.0004,

0.1277,-0.2633,0.1298,0.4853,0.6942,0.9046,0.3005,-0.3024,
0.1274,-0.2634,0.0009,0.4531,0.694,0.5766,0.577,-0.5784,
-0.001,-0.2629,0.1297,0.4855,0.662,0.5772,0.5757,-0.5791,

0.1275,0.6543,0.1295,0.7349,0.8114,0.706,0.7082,0.0003,
0.1275,0.6541,0.0009,0.7623,0.8284,0.5775,0.5768,-0.5777,
-0.001,0.6544,0.1297,0.7179,0.8387,-0.0008,1,0.0006,

-0.1296,0.2646,0.0009,0.8109,0.9347,-0.5785,-0.5744,-0.5791,
-0.0011,0.2642,0.0009,0.828,0.9075,-0.0008,-0.7053,-0.7089,
-0.001,0.2634,0.1297,0.8554,0.9246,0.0003,-0.7048,-0.7094,

0.1279,-0.2629,0.258,0.0948,0.8366,0.7079,-0.0001,0.7063,
-0.0008,-0.0009,0.2582,0.0708,0.7675,0.0003,0,1,
-0.0008,-0.2629,0.2582,0.1175,0.8136,0.0003,-0.0001,1,

-0.1296,-0.2629,0.0009,0.1853,0.7447,-0.0001,0.0001,-1,
-0.1296,-0.0017,0.0009,0.1387,0.6989,0.0005,0.7045,-0.7097,
-0.0015,-0.0019,0.0009,0.1612,0.676,0.5766,0.574,-0.5814,

-0.1296,-0.2629,0.0009,0.1853,0.7447,-0.0001,0.0001,-1,
-0.0015,-0.0019,0.0009,0.1612,0.676,0.5766,0.574,-0.5814,
-0.0014,-0.2636,0.0009,0.208,0.7219,0.2994,0.3001,-0.9057,

-0.1296,-0.0007,0.13,0.1157,0.6762,-0.8162,0.4066,-0.4105,
-0.0009,-0.0007,0.1296,0.1384,0.6533,0.3002,0.3005,-0.9053,
-0.1296,-0.0017,0.0009,0.1387,0.6989,0.0005,0.7045,-0.7097,

-0.0009,-0.0007,0.1296,0.1836,0.6528,0.3002,0.3005,-0.9053,
-0.001,-0.2629,0.1297,0.2304,0.6988,0.5772,0.5757,-0.5791,
-0.0014,-0.2636,0.0009,0.208,0.7219,0.2994,0.3001,-0.9057,

-0.0009,-0.0007,0.1296,0.1836,0.6528,0.3002,0.3005,-0.9053,
-0.0014,-0.2636,0.0009,0.208,0.7219,0.2994,0.3001,-0.9057,
-0.0015,-0.0019,0.0009,0.1612,0.676,0.5766,0.574,-0.5814,

-0.1296,-0.2629,0.1298,0.1627,0.7677,-1,0,-0.0001,
-0.1296,-0.0017,0.0009,0.1387,0.6989,-1,0,-0.0001,
-0.1296,-0.2629,0.0009,0.1853,0.7447,-1,0,-0.0001,

-0.1296,-0.2629,0.0009,0.3887,0.6933,-1,0,-0.0001,
-0.1296,-0.6536,0.1298,0.3556,0.7908,-1,0,-0.0001,
-0.1296,-0.2629,0.1298,0.3564,0.693,-1,0,-0.0001,

-0.1296,-0.2629,0.1298,0.1627,0.7677,-1,0,0,
-0.1296,-0.0007,0.13,0.1159,0.7217,-1,0,0,
-0.1296,-0.0017,0.0009,0.1387,0.6989,-1,0,0,

-0.1296,-0.0007,0.13,0.1159,0.7217,-0.8162,0.4066,-0.4105,
-0.1296,-0.2629,0.1298,0.1627,0.7677,-1,0,0.0001,
-0.1296,-0.2629,0.2581,0.1401,0.7906,-0.7072,-0.0001,0.707,

-0.1296,-0.6535,0.2581,0.3234,0.7905,-0.5779,-0.5763,0.5779,
-0.1296,-0.2629,0.2581,0.3243,0.6927,-0.7072,-0.0001,0.707,
-0.1296,-0.2629,0.1298,0.3564,0.693,-1,0,0.0001,

-0.1296,-0.2629,0.0009,0.3887,0.6933,-1,0,0,
-0.1296,-0.653,0.0009,0.3878,0.791,-1,0,0,
-0.1296,-0.6536,0.1298,0.3556,0.7908,-1,0,0,

-0.0009,-0.0007,0.1296,0.1384,0.6533,0.3002,0.3005,-0.9053,
-0.0015,-0.0019,0.0009,0.1612,0.676,0.5766,0.574,-0.5814,
-0.1296,-0.0017,0.0009,0.1387,0.6989,0.0005,0.7045,-0.7097,

-0.001,0.2634,0.1297,0.8554,0.9246,0.0003,-0.7048,-0.7094,
-0.0011,0.2642,0.0009,0.828,0.9075,-0.0008,-0.7053,-0.7089,
0.1274,0.2643,0.0009,0.845,0.8802,0.5776,-0.5758,-0.5787,

-0.001,0.2634,0.1297,0.8554,0.9246,0.0003,-0.7048,-0.7094,
-0.1296,0.2637,0.1299,0.8382,0.9519,-0.9042,-0.301,-0.3029,
-0.1296,0.2646,0.0009,0.8109,0.9347,-0.5785,-0.5744,-0.5791,

0.1274,-0.6535,0.0009,0.4522,0.7916,0.5772,-0.5763,-0.5785,
0.1274,-0.2634,0.0009,0.4531,0.694,0.5766,0.577,-0.5784,
0.1277,-0.2633,0.1298,0.4853,0.6942,0.9046,0.3005,-0.3024,

0.1274,-0.2634,0.0009,0.4531,0.694,0.5766,0.577,-0.5784,
-0.0014,-0.2636,0.0009,0.4532,0.6617,0.2994,0.3001,-0.9057,
-0.001,-0.2629,0.1297,0.4855,0.662,0.5772,0.5757,-0.5791,

0.1277,-0.2633,0.1298,0.0723,0.8595,0.9046,0.3005,-0.3024,
0.1276,-0.0009,0.1301,0.0256,0.8133,0.708,0.0004,-0.7062,
0.1279,-0.0009,0.258,0.0481,0.7905,0.7081,0.0001,0.7061,

0.1276,-0.0009,0.1301,0.9184,0.8882,0.708,0.0004,-0.7062,
0.1278,0.2643,0.1298,0.8621,0.8529,0.905,-0.3006,-0.3009,
0.1279,0.2634,0.258,0.8794,0.8258,0.7077,-0.0001,0.7065,

0.1274,0.2643,0.0009,0.845,0.8802,0.5776,-0.5758,-0.5787,
0.1275,0.6541,0.0009,0.7623,0.8284,0.5775,0.5768,-0.5777,
0.1275,0.6543,0.1295,0.7794,0.8011,0.706,0.7082,0.0003,

0.1278,0.2643,0.1298,0.8621,0.8529,0.905,-0.3006,-0.3009,
0.1275,0.6543,0.1295,0.7794,0.8011,0.706,0.7082,0.0003,
0.1278,0.6536,0.258,0.7967,0.7739,0.5789,0.5752,0.578,

0.1275,0.6541,0.0009,0.7623,0.8284,0.5775,0.5768,-0.5777,
-0.0011,0.6539,0.0009,0.7453,0.8558,-0.0007,0.7058,-0.7085,
-0.001,0.6544,0.1297,0.7179,0.8387,-0.0008,1,0.0006,

-0.001,0.6544,0.1297,0.7179,0.8387,-0.0008,1,0.0006,
-0.0011,0.6539,0.0009,0.7453,0.8558,-0.0007,0.7058,-0.7085,
-0.1296,0.6539,0.0009,0.7282,0.8831,-0.5779,0.5765,-0.5777,

-0.001,0.6544,0.1297,0.7179,0.8387,-0.0008,1,0.0006,
-0.1296,0.6539,0.1298,0.7009,0.866,-0.7081,0.7061,0.0004,
-0.1296,0.6537,0.2581,0.6736,0.849,-0.578,0.576,0.578,

-0.001,0.6544,0.1297,0.7179,0.8387,-0.0008,1,0.0006,
-0.0008,0.6539,0.2581,0.6906,0.8216,0.0006,0.7064,0.7078,
0.1278,0.6536,0.258,0.7077,0.7943,0.5789,0.5752,0.578,

0.1274,0.2643,0.0009,0.845,0.8802,0.5776,-0.5758,-0.5787,
-0.0011,0.2642,0.0009,0.828,0.9075,-0.0008,-0.7053,-0.7089,
-0.0011,0.6539,0.0009,0.7453,0.8558,-0.0007,0.7058,-0.7085,

-0.1296,0.2646,0.0009,0.8109,0.9347,-0.5785,-0.5744,-0.5791,
-0.1296,0.6539,0.0009,0.7282,0.8831,-0.5779,0.5765,-0.5777,
-0.0011,0.6539,0.0009,0.7453,0.8558,-0.0007,0.7058,-0.7085,

-0.1296,0.6539,0.0009,0.8652,0.6647,-0.5779,0.5765,-0.5777,
-0.1296,0.2646,0.0009,0.9477,0.7166,-0.5785,-0.5744,-0.5791,
-0.1296,0.2637,0.1299,0.9307,0.744,-0.9042,-0.301,-0.3029,

-0.1296,0.2634,0.2581,0.9137,0.7712,-0.7072,0.0001,0.707,
-0.0008,0.2634,0.2582,0.8966,0.7986,0.0003,0.0001,1,
-0.0008,0.6539,0.2581,0.8138,0.7465,0.0006,0.7064,0.7078,

0.1279,0.2634,0.258,0.8794,0.8258,0.7077,-0.0001,0.7065,
0.1278,0.6536,0.258,0.7967,0.7739,0.5789,0.5752,0.578,
-0.0008,0.6539,0.2581,0.8138,0.7465,0.0006,0.7064,0.7078,

0.1279,-0.0009,0.258,0.9354,0.861,0.7081,0.0001,0.7061,
0.1279,0.2634,0.258,0.8794,0.8258,0.7077,-0.0001,0.7065,
-0.0008,0.2634,0.2582,0.8966,0.7986,0.0003,0.0001,1,

-0.1296,-0.0009,0.2581,0.9697,0.8065,-0.7072,0,0.707,
-0.0008,-0.0009,0.2582,0.9526,0.8338,0.0003,0,1,
-0.0008,0.2634,0.2582,0.8966,0.7986,0.0003,0.0001,1,

0.1279,-0.2629,0.258,0.0948,0.8366,0.7079,-0.0001,0.7063,
0.1279,-0.0009,0.258,0.0481,0.7905,0.7081,0.0001,0.7061,
-0.0008,-0.0009,0.2582,0.0708,0.7675,0.0003,0,1,

-0.0008,-0.0009,0.2582,0.0708,0.7675,0.0003,0,1,
-0.1296,-0.0009,0.2581,0.0934,0.7446,-0.7072,0,0.707,
-0.1296,-0.2629,0.2581,0.1401,0.7906,-0.7072,-0.0001,0.707,

0.1279,-0.2629,0.258,0.4509,0.9538,0.7079,-0.0001,0.7063,
-0.0008,-0.2629,0.2582,0.4186,0.9535,0.0003,-0.0001,1,
-0.0008,-0.6536,0.2581,0.4194,0.8557,0.0006,-0.7059,0.7083,

-0.0008,-0.6536,0.2581,0.4194,0.8557,0.0006,-0.7059,0.7083,
-0.0008,-0.2629,0.2582,0.4186,0.9535,0.0003,-0.0001,1,
-0.1296,-0.2629,0.2581,0.3864,0.9532,-0.7072,-0.0001,0.707,

-0.1296,-0.6535,0.2581,0.3872,0.8554,-0.5779,-0.5763,0.5779,
-0.1296,-0.6536,0.1298,0.3875,0.8233,-0.45,-0.893,-0.0018,
-0.001,-0.6541,0.1297,0.4197,0.8236,-0.0004,-1,-0.0004,

-0.001,-0.6541,0.1297,0.4197,0.8236,-0.0004,-1,-0.0004,
-0.1296,-0.6536,0.1298,0.3875,0.8233,-0.45,-0.893,-0.0018,
-0.1296,-0.653,0.0009,0.3878,0.791,-0.0032,-0.7045,-0.7097,

-0.001,-0.6541,0.1297,0.4197,0.8236,-0.0004,-1,-0.0004,
-0.0011,-0.6536,0.0009,0.42,0.7914,-0.0015,-0.7063,-0.708,
0.1274,-0.6535,0.0009,0.4522,0.7916,0.5772,-0.5763,-0.5785,

0.1278,-0.6533,0.258,0.4516,0.856,0.5785,-0.5757,0.5778,
-0.0008,-0.6536,0.2581,0.4194,0.8557,0.0006,-0.7059,0.7083,
-0.001,-0.6541,0.1297,0.4197,0.8236,-0.0004,-1,-0.0004,
		},
		84,
		{
			{
				vec3(-3.889,   19.615, 0.028) ,
				vec3(3.826,   19.622 ,0.027) ,
				vec3(3.835 ,  19.607, 7.741) ,
				vec3(-3.889 ,   19.612, 7.742) ,
				vec3(-3.888 ,    -19.604, 7.742) ,
				vec3(3.835,    -19.6, 7.74) ,
				vec3(3.823,  -19.604, 0.026) ,
				vec3(-3.889,  -19.589, 0.027)
			}
		},
		{ vec3(0,-2.2f,0) }
	);//正视图竖前面
	ConfigModel(model1);

	Model model2
	(
		{ 0.131,-0.2555,0.1278,0.9634,0.92,-0.7071,-0.0001,0.7071,
0.131,-0.1293,-0.0031,0.9318,0.8872,0,0.7071,0.7071,
0.131,-0.2555,-0.0031,0.9634,0.8872,-0.7071,0,0.7072,

0.131,-0.1293,-0.0031,0.2613,0.4793,0,0.7071,0.7071,
-0.1277,-0.1293,-0.0031,0.1975,0.4903,0,0.7069,0.7073,
-0.1276,-0.2556,-0.0031,0.1921,0.4591,0.7066,0.0005,0.7076,

0.131,-0.1293,-0.0031,0.2613,0.4793,0,0.7071,0.7071,
-0.1276,-0.2556,-0.0031,0.1921,0.4591,0.7066,0.0005,0.7076,
0.131,-0.2555,-0.0031,0.256,0.4482,-0.7071,0,0.7072,

-0.6526,-0.2555,0.1278,0.0671,0.5104,-0.7065,-0.0001,0.7077,
-0.2645,-0.1293,0.1278,0.1697,0.5357,0.3016,0.3015,0.9045,
-0.6527,0.0005,0.1278,0.0705,0.5726,-0.5772,0.5773,0.5776,

-0.6526,-0.2555,0.1278,0.0671,0.5104,-0.7065,-0.0001,0.7077,
-0.1277,-0.2555,0.1278,0.1958,0.4929,0.7072,0,0.707,
-0.2645,-0.1293,0.1278,0.1697,0.5357,0.3016,0.3015,0.9045,

-0.2645,-0.1293,0.1278,0.1697,0.5357,0.3016,0.3015,0.9045,
-0.1277,-0.1293,-0.0031,0.2188,0.5379,0,0.7069,0.7073,
-0.2645,-0.1293,-0.1284,0.2193,0.5808,0.7067,0.7075,0.0001,

-0.1276,-0.1293,0.1278,0.2012,0.5202,0.5778,0.5771,0.5771,
-0.1277,-0.1293,-0.0031,0.2188,0.5379,0,0.7069,0.7073,
-0.2645,-0.1293,0.1278,0.1697,0.5357,0.3016,0.3015,0.9045,

-0.2646,0.0005,-0.1284,0.1426,0.369,0.7068,0.7074,0.0001,
-0.6529,0.0005,-0.1284,0.0467,0.3847,-0.7074,0.7068,0.0008,
-0.6527,0.0005,0.1278,0.0363,0.3215,-0.5772,0.5773,0.5776,

0.1309,0.0005,-0.1284,0.4019,0.003,0.7071,0.7071,0,
-0.1278,0.0005,-0.0031,0.4653,0.0369,-0.5776,0.5771,0.5773,
0.1309,0.0005,-0.0031,0.4006,0.0343,0.5773,0.5774,0.5773,

0.266,0.0005,-0.1284,0.9206,0.4542,-0.5776,0.5773,-0.5772,
0.6533,0.0005,0.1277,0.8239,0.5187,0.5773,0.5774,0.5773,
0.6532,0.0005,-0.1284,0.8237,0.4545,0.5772,0.5774,-0.5774,

0.131,-0.1293,-0.0031,0.9871,0.4854,0,0.7071,0.7071,
0.131,-0.1293,0.1278,0.9872,0.5182,-0.5773,0.5773,0.5774,
0.2662,-0.1293,0.1278,0.9533,0.5183,-0.3014,0.3012,0.9047,

0.2661,-0.1293,-0.1284,0.9531,0.4541,-0.5774,0.5774,-0.5773,
0.131,-0.1293,-0.0031,0.9871,0.4854,0,0.7071,0.7071,
0.2662,-0.1293,0.1278,0.9533,0.5183,-0.3014,0.3012,0.9047,

0.131,-0.1293,-0.0031,0.9871,0.4854,0,0.7071,0.7071,
0.2661,-0.1293,-0.1284,0.9531,0.4541,-0.5774,0.5774,-0.5773,
0.131,-0.1293,-0.1284,0.987,0.454,0.707,0.7072,0,

-0.1277,-0.1293,-0.0031,0.2338,0.517,0,0.7069,0.7073,
-0.1276,-0.1293,0.1278,0.2012,0.5202,0.5778,0.5771,0.5771,
-0.1277,-0.2555,0.1278,0.1958,0.4929,0.7072,0,0.707,

-0.6529,0.0005,-0.1284,0.0068,0.578,-0.7074,0.7068,0.0008,
-0.6529,-0.1293,-0.1284,0.0049,0.5457,-1,0,0.0012,
-0.6528,-0.2555,-0.0031,0.0343,0.5122,-1,0.0001,0.0015,

-0.2645,0.0005,0.1278,0.1601,0.5669,0.5774,0.5774,0.5773,
-0.2645,-0.1293,0.1278,0.1697,0.5357,0.3016,0.3015,0.9045,
-0.2646,0.0005,-0.1284,0.2041,0.6022,0.7068,0.7074,0.0001,

-0.1278,0.0005,-0.1284,0.9808,0.2876,-0.7073,0.7069,0.0003,
-0.1278,-0.1293,-0.1284,0.9484,0.2895,-0.7069,0.7073,0.0006,
-0.1277,-0.1293,-0.0031,0.9466,0.2582,0,0.7069,0.7073,

0.6533,-0.1293,-0.1284,0.4427,0.3614,0.8943,0.0001,-0.4475,
0.2661,-0.1293,-0.1284,0.3665,0.3015,-0.5774,0.5774,-0.5773,
0.6532,0.0005,-0.1284,0.4628,0.3359,0.5772,0.5774,-0.5774,

0.1309,0.0005,-0.0031,0.9864,0.3836,0.5773,0.5774,0.5773,
0.131,-0.1293,-0.0031,0.954,0.3855,0,0.7071,0.7071,
0.131,-0.1293,-0.1284,0.9521,0.3542,0.707,0.7072,0,

0.1309,0.0005,-0.0031,0.7354,0.0355,0.5773,0.5774,0.5773,
-0.1278,0.0005,-0.0031,0.6706,0.0355,-0.5776,0.5771,0.5773,
-0.1277,-0.1293,-0.0031,0.6707,0.003,0,0.7069,0.7073,

0.1309,0.0005,-0.0031,0.7354,0.0355,0.5773,0.5774,0.5773,
-0.1277,-0.1293,-0.0031,0.6707,0.003,0,0.7069,0.7073,
0.131,-0.1293,-0.0031,0.7354,0.003,0,0.7071,0.7071,

0.6533,-0.1293,-0.1284,0.7912,0.4546,0.8943,0.0001,-0.4475,
0.6534,-0.1293,0.1277,0.7914,0.5187,0.7074,0,0.7068,
0.6532,-0.2555,-0.1284,0.7596,0.4546,1,-0.0002,-0.0004,

0.6533,0.0005,0.1277,0.8239,0.5187,0.5773,0.5774,0.5773,
0.6534,-0.1293,0.1277,0.7914,0.5187,0.7074,0,0.7068,
0.6532,0.0005,-0.1284,0.8237,0.4545,0.5772,0.5774,-0.5774,

0.6533,-0.2555,0.1277,0.3835,0.4367,0.7071,-0.0002,0.7071,
0.2662,-0.1293,0.1278,0.2877,0.4016,-0.3014,0.3012,0.9047,
0.131,-0.2555,0.1278,0.2807,0.3558,-0.7071,-0.0001,0.7071,

0.2661,0.0005,0.1278,0.2676,0.4271,-0.5773,0.5773,0.5774,
0.2662,-0.1293,0.1278,0.2877,0.4016,-0.3014,0.3012,0.9047,
0.6534,-0.1293,0.1277,0.3639,0.4616,0.7074,0,0.7068,

0.6533,0.0005,0.1277,0.3438,0.4871,0.5773,0.5774,0.5773,
0.2661,0.0005,0.1278,0.2676,0.4271,-0.5773,0.5773,0.5774,
0.6534,-0.1293,0.1277,0.3639,0.4616,0.7074,0,0.7068,

0.2661,0.0005,0.1278,0.9208,0.5184,-0.5773,0.5773,0.5774,
0.2661,-0.1293,-0.1284,0.9531,0.4541,-0.5774,0.5774,-0.5773,
0.2662,-0.1293,0.1278,0.9533,0.5183,-0.3014,0.3012,0.9047,

-0.1278,0.0005,-0.1284,0.9808,0.2876,0,0.0001,-1,
0.1309,0.0005,-0.1284,0.9846,0.3523,0,0.0001,-1,
-0.1278,-0.1293,-0.1284,0.9484,0.2895,0,0.0001,-1,

0.6533,-0.2555,-0.0031,0.4037,0.411,0,-1,0,
0.6533,-0.2555,0.1277,0.3835,0.4367,0,-1,0,
0.131,-0.2555,0.1278,0.2807,0.3558,0,-1,0,

-0.2645,-0.1293,0.1278,0.1697,0.5357,0.3016,0.3015,0.9045,
-0.2645,0.0005,0.1278,0.1601,0.5669,0.5774,0.5774,0.5773,
-0.6527,0.0005,0.1278,0.0705,0.5726,-0.5772,0.5773,0.5776,

-0.1277,-0.2555,0.1278,0.1958,0.4929,0.7072,0,0.707,
-0.1276,-0.1293,0.1278,0.2012,0.5202,0.5778,0.5771,0.5771,
-0.2645,-0.1293,0.1278,0.1697,0.5357,0.3016,0.3015,0.9045,

-0.1277,-0.2555,0.1278,0.1958,0.4929,0.7072,0,0.707,
-0.1276,-0.2556,-0.0031,0.2277,0.4861,0.7066,0.0005,0.7076,
-0.1277,-0.1293,-0.0031,0.2338,0.517,0,0.7069,0.7073,

-0.2645,-0.1293,0.1278,0.1697,0.5357,0.3016,0.3015,0.9045,
-0.2645,-0.1293,-0.1284,0.2193,0.5808,0.7067,0.7075,0.0001,
-0.2646,0.0005,-0.1284,0.2041,0.6022,0.7068,0.7074,0.0001,

-0.1277,-0.1293,-0.0031,0.4978,0.0383,0,0.7069,0.7073,
-0.1278,0.0005,-0.0031,0.4653,0.0369,-0.5776,0.5771,0.5773,
-0.1278,0.0005,-0.1284,0.4666,0.0056,-0.7073,0.7069,0.0003,

-0.6526,-0.2555,0.1278,0.0671,0.5104,-0.7065,-0.0001,0.7077,
-0.6527,0.0005,0.1278,0.0705,0.5726,-0.5772,0.5773,0.5776,
-0.6529,0.0005,-0.1284,0.0068,0.578,-0.7074,0.7068,0.0008,

0.131,-0.2555,0.1278,0.9634,0.92,-0.7071,-0.0001,0.7071,
0.131,-0.1293,0.1278,0.9318,0.92,-0.5773,0.5773,0.5774,
0.131,-0.1293,-0.0031,0.9318,0.8872,0,0.7071,0.7071,

0.2662,-0.1293,0.1278,0.2877,0.4016,-0.3014,0.3012,0.9047,
0.131,-0.1293,0.1278,0.2611,0.3807,-0.5773,0.5773,0.5774,
0.131,-0.2555,0.1278,0.2807,0.3558,-0.7071,-0.0001,0.7071,

0.2662,-0.1293,0.1278,0.2877,0.4016,-0.3014,0.3012,0.9047,
0.6533,-0.2555,0.1277,0.3835,0.4367,0.7071,-0.0002,0.7071,
0.6534,-0.1293,0.1277,0.3639,0.4616,0.7074,0,0.7068,

0.6534,-0.1293,0.1277,0.7914,0.5187,0.7074,0,0.7068,
0.6533,-0.1293,-0.1284,0.7912,0.4546,0.8943,0.0001,-0.4475,
0.6532,0.0005,-0.1284,0.8237,0.4545,0.5772,0.5774,-0.5774,

0.6534,-0.1293,0.1277,0.7914,0.5187,0.7074,0,0.7068,
0.6533,-0.2555,0.1277,0.7597,0.5188,0.7071,-0.0002,0.7071,
0.6533,-0.2555,-0.0031,0.7597,0.486,1,-0.0004,-0.0003,

0.266,0.0005,-0.1284,0.9206,0.4542,-0.5776,0.5773,-0.5772,
0.2661,0.0005,0.1278,0.9208,0.5184,-0.5773,0.5773,0.5774,
0.6533,0.0005,0.1277,0.8239,0.5187,0.5773,0.5774,0.5773,

-0.1277,-0.1293,-0.0031,0.2188,0.5379,0,0.7069,0.7073,
-0.1278,-0.1293,-0.1284,0.2422,0.557,-0.7069,0.7073,0.0006,
-0.2645,-0.1293,-0.1284,0.2193,0.5808,0.7067,0.7075,0.0001,

-0.6527,0.0005,0.1278,0.0363,0.3215,-0.5772,0.5773,0.5776,
-0.2645,0.0005,0.1278,0.1321,0.3056,0.5774,0.5774,0.5773,
-0.2646,0.0005,-0.1284,0.1426,0.369,0.7068,0.7074,0.0001,

0.131,-0.1293,-0.1284,0.9521,0.3542,0.707,0.7072,0,
0.1309,0.0005,-0.1284,0.9846,0.3523,0.7071,0.7071,0,
0.1309,0.0005,-0.0031,0.9864,0.3836,0.5773,0.5774,0.5773,

0.1309,0.0005,-0.1284,0.4019,0.003,0.7071,0.7071,0,
-0.1278,0.0005,-0.1284,0.4666,0.0056,-0.7073,0.7069,0.0003,
-0.1278,0.0005,-0.0031,0.4653,0.0369,-0.5776,0.5771,0.5773,

0.2661,-0.1293,-0.1284,0.3665,0.3015,-0.5774,0.5774,-0.5773,
0.266,0.0005,-0.1284,0.3865,0.276,-0.5776,0.5773,-0.5772,
0.6532,0.0005,-0.1284,0.4628,0.3359,0.5772,0.5774,-0.5774,

-0.6529,0.0005,-0.1284,0.0467,0.3847,0,0,-1,
-0.2646,0.0005,-0.1284,0.1426,0.369,0,0,-1,
-0.2645,-0.1293,-0.1284,0.148,0.4013,0,0,-1,

0.1309,0.0005,-0.1284,0.9846,0.3523,-0.0001,0.0002,-1,
0.131,-0.1293,-0.1284,0.9521,0.3542,-0.0001,0.0002,-1,
-0.1278,-0.1293,-0.1284,0.9484,0.2895,-0.0001,0.0002,-1,

0.2661,0.0005,0.1278,0.9208,0.5184,-0.5773,0.5773,0.5774,
0.266,0.0005,-0.1284,0.9206,0.4542,-0.5776,0.5773,-0.5772,
0.2661,-0.1293,-0.1284,0.9531,0.4541,-0.5774,0.5774,-0.5773,

-0.1276,-0.2556,-0.0031,0.1921,0.4591,-0.0001,-1,0,
-0.6526,-0.2555,0.1278,0.0671,0.5104,-0.0001,-1,0,
-0.6528,-0.2555,-0.0031,0.0619,0.4785,-0.0001,-1,0,

0.131,-0.2555,-0.0031,0.3009,0.3301,0.0001,-1,0,
0.6532,-0.2555,-0.1284,0.4231,0.3863,0.0001,-1,0,
0.6533,-0.2555,-0.0031,0.4037,0.411,0.0001,-1,0,

-0.6528,-0.2555,-0.0031,0.0619,0.4785,0,-1,0,
-0.6529,-0.2555,-0.1284,0.0569,0.4477,0,-1,0,
-0.0027,-0.2555,-0.1284,0.2178,0.4227,0,-1,0,

0.6533,-0.1293,-0.1284,0.4427,0.3614,-0.0001,0,-1,
0.6532,-0.2555,-0.1284,0.4231,0.3863,-0.0001,0,-1,
0.131,-0.2555,-0.1284,0.3203,0.3054,-0.0001,0,-1,

0.131,-0.1293,-0.1284,0.3399,0.2806,-0.0001,-0.0001,-1,
0.2661,-0.1293,-0.1284,0.3665,0.3015,-0.0001,-0.0001,-1,
0.131,-0.2555,-0.1284,0.3203,0.3054,-0.0001,-0.0001,-1,

0.2661,-0.1293,-0.1284,0.3665,0.3015,0,-0.0002,-1,
0.6533,-0.1293,-0.1284,0.4427,0.3614,0,-0.0002,-1,
0.131,-0.2555,-0.1284,0.3203,0.3054,0,-0.0002,-1,

0.131,-0.2555,-0.1284,0.2508,0.4173,-0.0001,-0.0001,-1,
-0.0027,-0.2555,-0.1284,0.2178,0.4227,-0.0001,-0.0001,-1,
0.131,-0.1293,-0.1284,0.2459,0.3861,-0.0001,-0.0001,-1,

-0.0027,-0.2555,-0.1284,0.2178,0.4227,-0.0001,-0.0002,-1,
-0.2645,-0.1293,-0.1284,0.148,0.4013,-0.0001,-0.0002,-1,
-0.1278,-0.1293,-0.1284,0.1819,0.3962,-0.0001,-0.0002,-1,

-0.0027,-0.2555,-0.1284,0.2178,0.4227,-0.0001,-0.0001,-1,
-0.1278,-0.1293,-0.1284,0.1819,0.3962,-0.0001,-0.0001,-1,
0.131,-0.1293,-0.1284,0.2459,0.3861,-0.0001,-0.0001,-1,

0.131,-0.2555,0.1278,0.2807,0.3558,0.0001,-1,0,
0.131,-0.2555,-0.0031,0.3009,0.3301,0.0001,-1,0,
0.6533,-0.2555,-0.0031,0.4037,0.411,0.0001,-1,0,

0.131,-0.2555,-0.0031,0.256,0.4482,0.0001,-1,-0.0001,
-0.1276,-0.2556,-0.0031,0.1921,0.4591,0.0001,-1,-0.0001,
0.131,-0.2555,-0.1284,0.2508,0.4173,0.0001,-1,-0.0001,

0.131,-0.2555,-0.0031,0.3009,0.3301,0,-1,-0.0001,
0.131,-0.2555,-0.1284,0.3203,0.3054,0,-1,-0.0001,
0.6532,-0.2555,-0.1284,0.4231,0.3863,0,-1,-0.0001,

-0.1276,-0.2556,-0.0031,0.1921,0.4591,0,-1,-0.0004,
-0.0027,-0.2555,-0.1284,0.2178,0.4227,0,-1,-0.0004,
0.131,-0.2555,-0.1284,0.2508,0.4173,0,-1,-0.0004,

-0.1276,-0.2556,-0.0031,0.1921,0.4591,0,-1,0.0005,
-0.1277,-0.2555,0.1278,0.1958,0.4929,0,-1,0.0005,
-0.6526,-0.2555,0.1278,0.0671,0.5104,0,-1,0.0005,

-0.0027,-0.2555,-0.1284,0.2178,0.4227,-0.0001,-1,-0.0005,
-0.1276,-0.2556,-0.0031,0.1921,0.4591,-0.0001,-1,-0.0005,
-0.6528,-0.2555,-0.0031,0.0619,0.4785,-0.0001,-1,-0.0005,

-0.2645,-0.1293,-0.1284,0.148,0.4013,-0.0001,0,-1,
-0.6529,-0.2555,-0.1284,0.0569,0.4477,-0.0001,0,-1,
-0.6529,-0.1293,-0.1284,0.0519,0.4167,-0.0001,0,-1,

-0.2645,-0.1293,-0.1284,0.148,0.4013,0,0,-1,
-0.0027,-0.2555,-0.1284,0.2178,0.4227,0,0,-1,
-0.6529,-0.2555,-0.1284,0.0569,0.4477,0,0,-1,

-0.2645,-0.1293,-0.1284,0.148,0.4013,-0.0001,0,-1,
-0.6529,-0.1293,-0.1284,0.0519,0.4167,-0.0001,0,-1,
-0.6529,0.0005,-0.1284,0.0467,0.3847,-0.0001,0,-1,

-0.6529,-0.1293,-0.1284,0.0049,0.5457,-1,0,0.0012,
-0.6529,-0.2555,-0.1284,0.003,0.5141,-1,0.0001,0.0013,
-0.6528,-0.2555,-0.0031,0.0343,0.5122,-1,0.0001,0.0015,

-0.6528,-0.2555,-0.0031,0.0343,0.5122,-1,0.0001,0.0015,
-0.6526,-0.2555,0.1278,0.0671,0.5104,-0.7065,-0.0001,0.7077,
-0.6529,0.0005,-0.1284,0.0068,0.578,-0.7074,0.7068,0.0008,

0.6533,-0.2555,-0.0031,0.7597,0.486,1,-0.0004,-0.0003,
0.6532,-0.2555,-0.1284,0.7596,0.4546,1,-0.0002,-0.0004,
0.6534,-0.1293,0.1277,0.7914,0.5187,0.7074,0,0.7068,
		},
		70,
		{
			{
				vec3(-19.588   ,0.014,-3.851) ,
				vec3(-19.58 , 0.014, 3.835) ,
				vec3(-19.577  , -7.665,3.835) ,
				vec3(-19.588  , -7.665, -3.851) ,
				vec3(19.599  , 0.014, 3.832) ,
				vec3(19.597, 0.014,  -3.853) ,
				vec3(19.597, -7.665,  -3.853) ,
				vec3(19.6 , -7.665,  3.832)
			}
		},
		{ vec3(0,0,-2.2f) }
	);//正视图横向下面
	ConfigModel(model2);

	Model model3
	(
		{ 0.6523,0.0032,0.1277,0.003,0.1123,0.5777,-0.5775,0.5768,
0.6518,0.0032,-0.1285,0.0667,0.1049,0.5764,-0.5777,-0.578,
0.652,0.2592,-0.1285,0.0741,0.1686,0.577,0.577,-0.578,

-0.6523,0.0032,0.1277,0.4549,0.0592,-0.5767,-0.5775,0.5779,
-0.6529,0.2592,-0.1285,0.3986,0.1303,-0.5781,0.5771,-0.5769,
-0.6529,0.0032,-0.1285,0.3911,0.0667,-0.578,-0.5772,-0.5768,

-0.6529,0.0032,-0.1285,0.3911,0.0667,-0.578,-0.5772,-0.5768,
-0.6529,0.2592,-0.1285,0.3986,0.1303,-0.5781,0.5771,-0.5769,
0.6518,0.0032,-0.1285,0.0667,0.1049,0.5764,-0.5777,-0.578,

-0.6523,0.0032,0.1277,0.3835,0.003,-0.5767,-0.5775,0.5779,
-0.6529,0.0032,-0.1285,0.3911,0.0667,-0.578,-0.5772,-0.5768,
0.6523,0.0032,0.1277,0.0591,0.0412,0.5777,-0.5775,0.5768,

-0.6523,0.2594,0.1277,0.406,0.1941,-0.5766,0.5776,0.5778,
0.6523,0.0032,0.1277,0.089,0.296,0.5777,-0.5775,0.5768,
0.6525,0.2594,0.1277,0.0815,0.2323,0.5783,0.5772,0.5766,

-0.6529,0.2592,-0.1285,0.3986,0.1303,-0.5781,0.5771,-0.5769,
-0.6523,0.2594,0.1277,0.406,0.1941,-0.5766,0.5776,0.5778,
0.652,0.2592,-0.1285,0.0741,0.1686,0.577,0.577,-0.578,

-0.6523,0.2594,0.1277,0.406,0.1941,-0.5766,0.5776,0.5778,
-0.6523,0.0032,0.1277,0.4135,0.2578,-0.5767,-0.5775,0.5779,
0.6523,0.0032,0.1277,0.089,0.296,0.5777,-0.5775,0.5768,

-0.6523,0.2594,0.1277,0.406,0.1941,-0.5766,0.5776,0.5778,
0.6525,0.2594,0.1277,0.0815,0.2323,0.5783,0.5772,0.5766,
0.652,0.2592,-0.1285,0.0741,0.1686,0.577,0.577,-0.578,

0.652,0.2592,-0.1285,0.0741,0.1686,0.577,0.577,-0.578,
0.6525,0.2594,0.1277,0.0104,0.176,0.5783,0.5772,0.5766,
0.6523,0.0032,0.1277,0.003,0.1123,0.5777,-0.5775,0.5768,

-0.6523,0.0032,0.1277,0.4549,0.0592,-0.5767,-0.5775,0.5779,
-0.6523,0.2594,0.1277,0.4623,0.1229,-0.5766,0.5776,0.5778,
-0.6529,0.2592,-0.1285,0.3986,0.1303,-0.5781,0.5771,-0.5769,

-0.6529,0.2592,-0.1285,0.3986,0.1303,-0.5781,0.5771,-0.5769,
0.652,0.2592,-0.1285,0.0741,0.1686,0.577,0.577,-0.578,
0.6518,0.0032,-0.1285,0.0667,0.1049,0.5764,-0.5777,-0.578,

-0.6529,0.0032,-0.1285,0.3911,0.0667,-0.578,-0.5772,-0.5768,
0.6518,0.0032,-0.1285,0.0667,0.1049,0.5764,-0.5777,-0.578,
0.6523,0.0032,0.1277,0.0591,0.0412,0.5777,-0.5775,0.5768,
		},
		12,
		{
			{
				vec3(-19.569 , 7.782, 3.831) ,
				vec3(-19.569   ,0.095,3.831) ,
				vec3(19.569 , 0.095, 3.83) ,
				vec3(19.577  , 7.782,3.83) ,
				vec3(19.561  ,7.777, -3.855) ,
				vec3(19.553 ,0.095, -3.855) ,
				vec3(-19.586  , 0.095, -3.854) ,
				vec3(-19.586 , 7.777,  -3.854)
				}
		},
		{ vec3(2.2f,0,0) }
	);//最简单的
	ConfigModel(model3);

	Model model4
	(
		{ 0.1255,0.6534,-0.2601,0.6951,0.6799,0.5701,0.5791,-0.5828,
-0.1291,0.6516,-0.2594,0.755,0.7016,-0.5776,0.5719,-0.5825,
0.1277,0.6538,-0.1324,0.6837,0.7098,0.7078,0.7064,-0.0082,

-0.1299,-0.6538,-0.1301,0.621,0.353,-0.7069,-0.7073,0.0047,
-0.13,-0.6527,-0.2585,0.622,0.3208,-0.5788,-0.5762,-0.5771,
0.1262,-0.6527,-0.2601,0.6861,0.3224,0.573,-0.5755,-0.5835,

0.1266,0.2625,-0.2612,0.3781,0.4923,0.7045,0.0025,-0.7097,
0.1272,-0.2639,-0.1345,0.509,0.457,0.9055,0.3,0.3001,
0.1266,-0.2625,-0.2612,0.5095,0.4887,0.7024,-0.0009,-0.7118,

-0.131,0.2625,-0.2589,0.3799,0.5568,-0.715,0.0023,-0.6991,
0.1266,-0.2625,-0.2612,0.5095,0.4887,0.7024,-0.0009,-0.7118,
-0.131,-0.2625,-0.2589,0.5113,0.5532,-0.7163,0,-0.6978,

-0.1294,0.2639,-0.1323,0.3804,0.5885,-0.9004,-0.2993,0.3159,
-0.131,-0.2625,-0.2589,0.5113,0.5532,-0.7163,0,-0.6978,
-0.1286,-0.2639,-0.1314,0.5125,0.5851,-0.8981,0.3036,0.3182,

0.1263,0.264,-0.1337,0.3821,0.6525,0.904,-0.3021,0.3025,
-0.1286,-0.2639,-0.1314,0.5125,0.5851,-0.8981,0.3036,0.3182,
0.1272,-0.2639,-0.1345,0.5142,0.6491,0.9055,0.3,0.3001,

0.1262,-0.6527,-0.2601,0.6861,0.3224,0.573,-0.5755,-0.5835,
0.1272,-0.2639,-0.1345,0.7203,0.226,0.9055,0.3,0.3001,
0.1277,-0.6538,-0.1324,0.7181,0.3236,0.7076,-0.7066,-0.0062,

0.1272,-0.2639,-0.1345,0.7203,0.226,0.9055,0.3,0.3001,
0.1262,-0.6527,-0.2601,0.6861,0.3224,0.573,-0.5755,-0.5835,
0.1266,-0.2625,-0.2612,0.6886,0.2247,0.7024,-0.0009,-0.7118,

0.1266,0.2625,-0.2612,0.7287,0.588,0.7045,0.0025,-0.7097,
0.1255,0.6534,-0.2601,0.6951,0.6799,0.5701,0.5791,-0.5828,
0.1263,0.264,-0.1337,0.6986,0.5775,0.904,-0.3021,0.3025,

-0.131,-0.2625,-0.2589,0.6241,0.2231,-0.7163,0,-0.6978,
0.1266,-0.2625,-0.2612,0.6886,0.2247,0.7024,-0.0009,-0.7118,
-0.13,-0.6527,-0.2585,0.622,0.3208,-0.5788,-0.5762,-0.5771,

0.1266,0.2625,-0.2612,0.7287,0.588,0.7045,0.0025,-0.7097,
-0.131,0.2625,-0.2589,0.7893,0.6104,-0.715,0.0023,-0.6991,
0.1255,0.6534,-0.2601,0.6951,0.6799,0.5701,0.5791,-0.5828,

-0.131,-0.2625,-0.2589,0.6241,0.2231,-0.7163,0,-0.6978,
-0.13,-0.6527,-0.2585,0.622,0.3208,-0.5788,-0.5762,-0.5771,
-0.1299,-0.6538,-0.1301,0.5898,0.3205,-0.7069,-0.7073,0.0047,

-0.1291,0.6516,-0.2594,0.755,0.7016,-0.5776,0.5719,-0.5825,
-0.1294,0.2639,-0.1323,0.8188,0.6219,-0.9004,-0.2993,0.3159,
-0.1299,0.6538,-0.1301,0.7852,0.7135,-0.7081,0.7061,-0.0008,

-0.1294,0.2639,-0.1323,0.8188,0.6219,-0.9004,-0.2993,0.3159,
-0.1291,0.6516,-0.2594,0.755,0.7016,-0.5776,0.5719,-0.5825,
-0.131,0.2625,-0.2589,0.7893,0.6104,-0.715,0.0023,-0.6991,

0.1284,-0.266,-0.0046,0.004,0.623,0.5849,0.5709,0.5762,
-0.1277,-0.2653,-0.0024,0.0682,0.6237,-0.5721,0.575,0.5849,
0.1277,-0.6534,-0.0046,0.003,0.7201,0.5798,-0.5769,0.5753,

-0.1284,0.266,-0.0024,0.9206,0.5649,-0.5743,-0.5716,0.586,
0.1284,0.6527,-0.0046,0.9868,0.6605,0.5836,0.5731,0.5752,
-0.1277,0.6534,-0.0024,0.9227,0.6619,-0.5697,0.5769,0.5853,

0.1277,-0.6534,-0.0046,0.6845,0.3864,0.5798,-0.5769,0.5753,
-0.1299,-0.6538,-0.1301,0.621,0.353,-0.7069,-0.7073,0.0047,
0.1277,-0.6538,-0.1324,0.6855,0.3544,0.7076,-0.7066,-0.0062,

-0.1284,-0.6527,-0.0024,0.5579,0.3195,-0.5731,-0.5734,0.5855,
-0.1277,-0.2653,-0.0024,0.5598,0.2226,-0.5721,0.575,0.5849,
-0.1299,-0.6538,-0.1301,0.5898,0.3205,-0.7069,-0.7073,0.0047,

-0.1277,-0.2653,-0.0024,0.0682,0.6237,-0.5721,0.575,0.5849,
0.1272,-0.2639,-0.1345,0.0044,0.5905,0.9055,0.3,0.3001,
-0.1286,-0.2639,-0.1314,0.0684,0.5914,-0.8981,0.3036,0.3182,

0.1277,-0.6534,-0.0046,0.7501,0.3243,0.5798,-0.5769,0.5753,
0.1277,-0.6538,-0.1324,0.7181,0.3236,0.7076,-0.7066,-0.0062,
0.1272,-0.2639,-0.1345,0.7203,0.226,0.9055,0.3,0.3001,

-0.1277,0.6534,-0.0024,0.7329,0.7621,-0.5697,0.5769,0.5853,
0.1277,0.6538,-0.1324,0.6837,0.7098,0.7078,0.7064,-0.0082,
-0.1299,0.6538,-0.1301,0.7443,0.7322,-0.7081,0.7061,-0.0008,

0.1284,0.6527,-0.0046,0.635,0.658,0.5836,0.5731,0.5752,
0.1263,0.264,-0.1337,0.6986,0.5775,0.904,-0.3021,0.3025,
0.1277,0.6538,-0.1324,0.665,0.6691,0.7078,0.7064,-0.0082,

0.1277,0.2653,-0.0046,0.9847,0.5635,0.5823,-0.5754,0.5744,
-0.1284,0.266,-0.0024,0.9206,0.5649,-0.5743,-0.5716,0.586,
0.1263,0.264,-0.1337,0.9839,0.5312,0.904,-0.3021,0.3025,

-0.1284,0.266,-0.0024,0.8491,0.6337,-0.5743,-0.5716,0.586,
-0.1277,0.6534,-0.0024,0.8152,0.7246,-0.5697,0.5769,0.5853,
-0.1294,0.2639,-0.1323,0.8188,0.6219,-0.9004,-0.2993,0.3159,

-0.1294,0.2639,-0.1323,0.3804,0.5885,-0.9004,-0.2993,0.3159,
-0.131,0.2625,-0.2589,0.3799,0.5568,-0.715,0.0023,-0.6991,
-0.131,-0.2625,-0.2589,0.5113,0.5532,-0.7163,0,-0.6978,

-0.1299,-0.6538,-0.1301,0.5898,0.3205,-0.7069,-0.7073,0.0047,
-0.1286,-0.2639,-0.1314,0.5921,0.2229,-0.8981,0.3036,0.3182,
-0.131,-0.2625,-0.2589,0.6241,0.2231,-0.7163,0,-0.6978,

-0.1277,-0.2653,-0.0024,0.5598,0.2226,-0.5721,0.575,0.5849,
-0.1286,-0.2639,-0.1314,0.5921,0.2229,-0.8981,0.3036,0.3182,
-0.1299,-0.6538,-0.1301,0.5898,0.3205,-0.7069,-0.7073,0.0047,

-0.1277,-0.2653,-0.0024,0.0682,0.6237,-0.5721,0.575,0.5849,
0.1284,-0.266,-0.0046,0.004,0.623,0.5849,0.5709,0.5762,
0.1272,-0.2639,-0.1345,0.0044,0.5905,0.9055,0.3,0.3001,

-0.1277,-0.2653,-0.0024,0.0682,0.6237,-0.5721,0.575,0.5849,
-0.1284,-0.6527,-0.0024,0.0671,0.7207,-0.5731,-0.5734,0.5855,
0.1277,-0.6534,-0.0046,0.003,0.7201,0.5798,-0.5769,0.5753,

-0.1284,0.266,-0.0024,0.9206,0.5649,-0.5743,-0.5716,0.586,
0.1277,0.2653,-0.0046,0.9847,0.5635,0.5823,-0.5754,0.5744,
0.1284,0.6527,-0.0046,0.9868,0.6605,0.5836,0.5731,0.5752,

0.1272,-0.2639,-0.1345,0.7203,0.226,0.9055,0.3,0.3001,
0.1284,-0.266,-0.0046,0.7528,0.2274,0.5849,0.5709,0.5762,
0.1277,-0.6534,-0.0046,0.7501,0.3243,0.5798,-0.5769,0.5753,

0.1266,0.2625,-0.2612,0.3781,0.4923,0.7045,0.0025,-0.7097,
0.1263,0.264,-0.1337,0.3769,0.4604,0.904,-0.3021,0.3025,
0.1272,-0.2639,-0.1345,0.509,0.457,0.9055,0.3,0.3001,

0.1263,0.264,-0.1337,0.3821,0.6525,0.904,-0.3021,0.3025,
-0.1294,0.2639,-0.1323,0.3804,0.5885,-0.9004,-0.2993,0.3159,
-0.1286,-0.2639,-0.1314,0.5125,0.5851,-0.8981,0.3036,0.3182,

-0.1284,0.266,-0.0024,0.9206,0.5649,-0.5743,-0.5716,0.586,
-0.1294,0.2639,-0.1323,0.9199,0.5324,-0.9004,-0.2993,0.3159,
0.1263,0.264,-0.1337,0.9839,0.5312,0.904,-0.3021,0.3025,

0.1277,-0.6534,-0.0046,0.6845,0.3864,0.5798,-0.5769,0.5753,
-0.1284,-0.6527,-0.0024,0.6204,0.3849,-0.5731,-0.5734,0.5855,
-0.1299,-0.6538,-0.1301,0.621,0.353,-0.7069,-0.7073,0.0047,

0.1262,-0.6527,-0.2601,0.6861,0.3224,0.573,-0.5755,-0.5835,
0.1277,-0.6538,-0.1324,0.6855,0.3544,0.7076,-0.7066,-0.0062,
-0.1299,-0.6538,-0.1301,0.621,0.353,-0.7069,-0.7073,0.0047,

-0.131,0.2625,-0.2589,0.7893,0.6104,-0.715,0.0023,-0.6991,
-0.1291,0.6516,-0.2594,0.755,0.7016,-0.5776,0.5719,-0.5825,
0.1255,0.6534,-0.2601,0.6951,0.6799,0.5701,0.5791,-0.5828,

-0.131,0.2625,-0.2589,0.3799,0.5568,-0.715,0.0023,-0.6991,
0.1266,0.2625,-0.2612,0.3781,0.4923,0.7045,0.0025,-0.7097,
0.1266,-0.2625,-0.2612,0.5095,0.4887,0.7024,-0.0009,-0.7118,

0.1284,0.6527,-0.0046,0.635,0.658,0.5836,0.5731,0.5752,
0.1277,0.2653,-0.0046,0.6681,0.5668,0.5823,-0.5754,0.5744,
0.1263,0.264,-0.1337,0.6986,0.5775,0.904,-0.3021,0.3025,

0.1255,0.6534,-0.2601,0.6951,0.6799,0.5701,0.5791,-0.5828,
0.1277,0.6538,-0.1324,0.665,0.6691,0.7078,0.7064,-0.0082,
0.1263,0.264,-0.1337,0.6986,0.5775,0.904,-0.3021,0.3025,

0.1266,-0.2625,-0.2612,0.6886,0.2247,0.7024,-0.0009,-0.7118,
0.1262,-0.6527,-0.2601,0.6861,0.3224,0.573,-0.5755,-0.5835,
-0.13,-0.6527,-0.2585,0.622,0.3208,-0.5788,-0.5762,-0.5771,

-0.1277,0.6534,-0.0024,0.8152,0.7246,-0.5697,0.5769,0.5853,
-0.1299,0.6538,-0.1301,0.7852,0.7135,-0.7081,0.7061,-0.0008,
-0.1294,0.2639,-0.1323,0.8188,0.6219,-0.9004,-0.2993,0.3159,

-0.1277,0.6534,-0.0024,0.7329,0.7621,-0.5697,0.5769,0.5853,
0.1284,0.6527,-0.0046,0.6727,0.7399,0.5836,0.5731,0.5752,
0.1277,0.6538,-0.1324,0.6837,0.7098,0.7078,0.7064,-0.0082,

-0.1291,0.6516,-0.2594,0.755,0.7016,-0.5776,0.5719,-0.5825,
-0.1299,0.6538,-0.1301,0.7443,0.7322,-0.7081,0.7061,-0.0008,
0.1277,0.6538,-0.1324,0.6837,0.7098,0.7078,0.7064,-0.0082,
		},
		44,
		{
			{
				vec3(-3.872  ,19.549, -7.782) ,
				vec3(3.766 , 19.601, -7.802) ,
				vec3(3.854 ,19.58, -0.139) ,
				vec3(-3.831  ,19.601, -0.072)  ,
				vec3(-3.898   ,-19.58,-7.756) ,
				vec3(-3.852  , -19.58, -0.071) ,
				vec3(3.832   , -19.601,-0.138) ,
				vec3(3.787 ,-19.58,  -7.802)
			}
		},
		{ vec3(-2.2f,0,0) }
	);//正视图竖后面
	ConfigModel(model4);

	Model model5
	(
		{ -0.1324,-0.1286,0.0016,0.8577,0.0724,0.7071,-0.7071,-0.0001,
-0.1324,-0.0013,0.0016,0.8895,0.0723,0.7071,0.7071,0,
-0.1324,-0.1286,0.1354,0.8577,0.1059,0.7071,-0.7071,-0.0002,

-0.1324,-0.0013,-0.2631,0.8053,0.329,0.7073,0,0.7069,
-0.1324,-0.0013,-0.132,0.8077,0.3617,0.9045,0.3016,0.3016,
-0.1324,-0.1286,-0.2631,0.7735,0.3313,0.3015,-0.9046,0.3013,

-0.1324,-0.1286,-0.1319,0.7759,0.3641,0.7071,-0.7071,0,
-0.1324,-0.1286,-0.2631,0.7735,0.3313,0.3015,-0.9046,0.3013,
-0.1324,-0.0013,-0.132,0.8077,0.3617,0.9045,0.3016,0.3016,

-0.1323,-0.0013,0.2646,0.6259,0.7918,0.7071,0,-0.7071,
-0.1324,-0.0013,0.1355,0.6253,0.7595,0.9044,0.3016,-0.3018,
-0.1324,0.1283,0.1355,0.6578,0.759,0.5772,0.5774,-0.5775,

-0.0032,0.1283,-0.2632,0.8353,0.2943,0.5775,0.5773,0.5772,
-0.0032,-0.0013,-0.2631,0.8029,0.2967,0.7074,0,0.7068,
-0.0033,-0.0013,-0.6525,0.7957,0.1995,0.7071,0,-0.7071,

-0.003,-0.1285,0.2646,0.5946,0.8248,0.5776,-0.5771,-0.5774,
-0.003,-0.0013,0.2646,0.6264,0.8242,0.7071,0,-0.7071,
-0.003,-0.0013,0.6547,0.628,0.9219,0.7071,0,0.7071,

-0.1324,-0.0013,-0.132,0.8894,0.0389,0.9045,0.3016,0.3016,
-0.1324,-0.0013,0.0016,0.8895,0.0723,0.7071,0.7071,0,
-0.1324,-0.1286,0.0016,0.8577,0.0724,0.7071,-0.7071,-0.0001,

-0.0033,-0.0013,-0.6525,0.7957,0.1995,0.7071,0,-0.7071,
-0.0032,-0.0013,-0.2631,0.8029,0.2967,0.7074,0,0.7068,
-0.0032,-0.1286,-0.2632,0.7712,0.2991,0.5776,-0.5773,0.5771,

-0.003,0.1283,0.6547,0.6604,0.9214,0.5774,0.5773,0.5773,
-0.003,-0.0013,0.6547,0.628,0.9219,0.7071,0,0.7071,
-0.003,0.1283,0.2646,0.6589,0.8237,0.5774,0.5774,-0.5772,

-0.1324,0.1283,-0.132,0.8401,0.3593,0.5773,0.5772,0.5775,
-0.1324,-0.0013,-0.132,0.8077,0.3617,0.9045,0.3016,0.3016,
-0.1324,0.1284,-0.2631,0.8377,0.3266,0.3015,0.9045,0.3015,

-0.1324,-0.1286,0.1354,0.5935,0.76,0.7071,-0.7071,-0.0002,
-0.1324,-0.0013,0.1355,0.6253,0.7595,0.9044,0.3016,-0.3018,
-0.1323,-0.0013,0.2646,0.6259,0.7918,0.7071,0,-0.7071,

-0.1324,-0.1286,0.2646,0.594,0.7924,0.3011,-0.9047,-0.3013,
-0.1324,-0.1286,0.1354,0.5935,0.76,0.7071,-0.7071,-0.0002,
-0.1323,-0.0013,0.2646,0.6259,0.7918,0.7071,0,-0.7071,

-0.2601,-0.1286,0.1354,0.9535,0.1056,-0.0003,-1,0.0001,
-0.1324,-0.1286,0.0016,0.9853,0.072,0.7071,-0.7071,-0.0001,
-0.1324,-0.1286,0.1354,0.9855,0.1055,0.7071,-0.7071,-0.0002,

-0.2601,-0.1286,0.0018,0.9534,0.0722,0,-1,0,
-0.2601,-0.1286,-0.1318,0.9533,0.0387,-0.0001,-1,0.0001,
-0.1324,-0.1286,0.0016,0.9853,0.072,0.7071,-0.7071,-0.0001,

-0.2601,-0.1286,-0.1318,0.6291,0.1476,-0.0001,-1,0.0001,
-0.1324,-0.1286,-0.2631,0.6643,0.177,0.3015,-0.9046,0.3013,
-0.1324,-0.1286,-0.1319,0.6315,0.1795,0.7071,-0.7071,0,

-0.2601,-0.1286,-0.2631,0.6618,0.1452,-0.0003,-1,-0.0001,
-0.2601,-0.1285,-0.6524,0.7591,0.1378,-0.0007,-0.707,-0.7072,
-0.1324,-0.1286,-0.2631,0.6643,0.177,0.3015,-0.9046,0.3013,

-0.2601,-0.0013,-0.6524,0.7908,0.1354,-0.4476,0,-0.8943,
-0.1324,-0.0013,-0.6525,0.7933,0.1673,-0.0002,0,-1,
-0.1324,-0.1286,-0.6525,0.7615,0.1697,-0.0001,-0.7073,-0.707,

-0.2601,-0.0013,-0.6524,0.7908,0.1354,-0.4476,0,-0.8943,
-0.1321,0.1284,-0.6525,0.8256,0.1649,-0.0001,0.7072,-0.707,
-0.1324,-0.0013,-0.6525,0.7933,0.1673,-0.0002,0,-1,

-0.2601,0.1284,-0.2631,0.9204,0.1254,-0.7071,0.7071,0.0001,
-0.1324,0.1284,-0.2631,0.9228,0.1573,0.3015,0.9045,0.3015,
-0.2601,0.1283,-0.6524,0.8232,0.1329,-0.5775,0.5773,-0.5772,

-0.2601,0.1283,-0.6524,0.8232,0.1329,-0.5775,0.5773,-0.5772,
-0.1324,0.1284,-0.2631,0.9228,0.1573,0.3015,0.9045,0.3015,
-0.1321,0.1284,-0.6525,0.8256,0.1649,-0.0001,0.7072,-0.707,

-0.2601,0.1283,-0.1319,0.9531,0.1228,-0.5772,0.5771,0.5778,
-0.1324,0.1283,-0.132,0.9556,0.1547,0.5773,0.5772,0.5775,
-0.1324,0.1284,-0.2631,0.9228,0.1573,0.3015,0.9045,0.3015,

-0.2601,0.1283,-0.1319,0.9531,0.1228,-0.5772,0.5771,0.5778,
-0.2601,-0.0013,-0.132,0.9854,0.1203,-0.5778,0.577,0.5773,
-0.1324,0.1283,-0.132,0.9556,0.1547,0.5773,0.5772,0.5775,

-0.2601,-0.0013,-0.132,0.9214,0.0387,-0.5778,0.577,0.5773,
-0.2601,-0.0013,0.0018,0.9215,0.0723,0.0001,1,0,
-0.1324,-0.0013,0.0016,0.8895,0.0723,0.7071,0.7071,0,

-0.2601,-0.0013,0.0018,0.9215,0.0723,0.0001,1,0,
-0.2601,-0.0013,0.1355,0.9216,0.1057,-0.5778,0.5771,-0.5772,
-0.1324,-0.0013,0.0016,0.8895,0.0723,0.7071,0.7071,0,

-0.26,0.1283,0.1355,0.6572,0.727,-0.5774,0.5772,-0.5774,
-0.1324,0.1283,0.1355,0.6578,0.759,0.5772,0.5774,-0.5775,
-0.1324,-0.0013,0.1355,0.6253,0.7595,0.9044,0.3016,-0.3018,

-0.26,0.1283,0.1355,0.3077,0.6588,-0.5774,0.5772,-0.5774,
-0.2601,0.1284,0.2646,0.3077,0.6911,-0.7071,0.7071,-0.0001,
-0.1324,0.1284,0.2646,0.2757,0.6911,0.3014,0.9046,-0.3015,

-0.2601,0.1284,0.2646,0.3077,0.6911,-0.7071,0.7071,-0.0001,
-0.1323,0.1284,0.6547,0.2757,0.7888,0,0.7072,0.7071,
-0.1324,0.1284,0.2646,0.2757,0.6911,0.3014,0.9046,-0.3015,

-0.26,0.1283,0.6548,0.6615,0.9858,-0.5774,0.5772,0.5774,
-0.26,-0.0013,0.6547,0.629,0.9863,-0.4473,0,0.8944,
-0.1323,0.1284,0.6547,0.661,0.9538,0,0.7072,0.7071,

-0.26,-0.0013,0.6547,0.629,0.9863,-0.4473,0,0.8944,
-0.26,-0.1285,0.6547,0.5972,0.9868,-0.0005,-0.7069,0.7073,
-0.1323,-0.1286,0.6547,0.5966,0.9548,-0.0002,-0.7072,0.707,

-0.26,-0.1285,0.2643,0.5907,0.4722,-0.0011,-1,0.0004,
-0.1324,-0.1286,0.2646,0.6227,0.4727,0.3011,-0.9047,-0.3013,
-0.26,-0.1285,0.6547,0.5895,0.57,-0.0005,-0.7069,0.7073,

-0.26,-0.1285,0.6547,0.5895,0.57,-0.0005,-0.7069,0.7073,
-0.1324,-0.1286,0.2646,0.6227,0.4727,0.3011,-0.9047,-0.3013,
-0.1323,-0.1286,0.6547,0.6215,0.5704,-0.0002,-0.7072,0.707,

-0.2601,-0.1286,0.1354,0.5912,0.44,-0.0003,-1,0.0001,
-0.1324,-0.1286,0.1354,0.6231,0.4404,0.7071,-0.7071,-0.0002,
-0.1324,-0.1286,0.2646,0.6227,0.4727,0.3011,-0.9047,-0.3013,

-0.26,-0.0013,0.6547,0.5577,0.5696,-0.4473,0,0.8944,
-0.26,0.1283,0.6548,0.5252,0.5692,-0.5774,0.5772,0.5774,
-0.2601,0.1284,0.2646,0.5264,0.4715,-0.7071,0.7071,-0.0001,

-0.2601,0.1284,0.2646,0.5264,0.4715,-0.7071,0.7071,-0.0001,
-0.26,0.1283,0.1355,0.5268,0.4392,-0.5774,0.5772,-0.5774,
-0.2601,-0.0013,0.1355,0.5593,0.4396,-0.5778,0.5771,-0.5772,

-0.2601,-0.0013,-0.132,0.7809,0.0055,-0.5778,0.577,0.5773,
-0.2601,0.1283,-0.1319,0.8133,0.003,-0.5772,0.5771,0.5778,
-0.2601,0.1284,-0.2631,0.8158,0.0357,-0.7071,0.7071,0.0001,

-0.2601,0.1284,-0.2631,0.8158,0.0357,-0.7071,0.7071,0.0001,
-0.2601,0.1283,-0.6524,0.8232,0.1329,-0.5775,0.5773,-0.5772,
-0.2601,-0.0013,-0.6524,0.7908,0.1354,-0.4476,0,-0.8943,

-0.1324,-0.0013,-0.2631,0.8053,0.329,0.7073,0,0.7069,
-0.1324,-0.1286,-0.2631,0.7735,0.3313,0.3015,-0.9046,0.3013,
-0.0032,-0.0013,-0.2631,0.8029,0.2967,0.7074,0,0.7068,

-0.1324,-0.1286,-0.2631,0.6643,0.177,0.3015,-0.9046,0.3013,
-0.0033,-0.1285,-0.6525,0.7639,0.2019,0.5773,-0.5772,-0.5775,
-0.0032,-0.1286,-0.2632,0.6667,0.2093,0.5776,-0.5773,0.5771,

-0.1324,-0.1286,-0.6525,0.7615,0.1697,-0.0001,-0.7073,-0.707,
-0.0033,-0.0013,-0.6525,0.7957,0.1995,0.7071,0,-0.7071,
-0.0033,-0.1285,-0.6525,0.7639,0.2019,0.5773,-0.5772,-0.5775,

-0.1321,0.1284,-0.6525,0.8256,0.1649,-0.0001,0.7072,-0.707,
-0.0033,0.1283,-0.6525,0.8281,0.1971,0.5772,0.5775,-0.5774,
-0.0033,-0.0013,-0.6525,0.7957,0.1995,0.7071,0,-0.7071,

-0.0032,0.1283,-0.2632,0.9253,0.1896,0.5775,0.5773,0.5772,
-0.0033,0.1283,-0.6525,0.8281,0.1971,0.5772,0.5775,-0.5774,
-0.1321,0.1284,-0.6525,0.8256,0.1649,-0.0001,0.7072,-0.707,

-0.1324,0.1284,-0.2631,0.8377,0.3266,0.3015,0.9045,0.3015,
-0.1324,-0.0013,-0.2631,0.8053,0.329,0.7073,0,0.7069,
-0.0032,-0.0013,-0.2631,0.8029,0.2967,0.7074,0,0.7068,

-0.1324,0.1284,0.2646,0.6583,0.7913,0.3014,0.9046,-0.3015,
-0.003,0.1283,0.2646,0.6589,0.8237,0.5774,0.5774,-0.5772,
-0.1323,-0.0013,0.2646,0.6259,0.7918,0.7071,0,-0.7071,

-0.1324,0.1284,0.2646,0.2757,0.6911,0.3014,0.9046,-0.3015,
-0.003,0.1283,0.6547,0.2433,0.7888,0.5774,0.5773,0.5773,
-0.003,0.1283,0.2646,0.2433,0.6911,0.5774,0.5774,-0.5772,

-0.1323,0.1284,0.6547,0.661,0.9538,0,0.7072,0.7071,
-0.003,-0.0013,0.6547,0.628,0.9219,0.7071,0,0.7071,
-0.003,0.1283,0.6547,0.6604,0.9214,0.5774,0.5773,0.5773,

-0.1323,-0.1286,0.6547,0.5966,0.9548,-0.0002,-0.7072,0.707,
-0.003,-0.1286,0.6547,0.5961,0.9224,0.5774,-0.5773,0.5773,
-0.003,-0.0013,0.6547,0.628,0.9219,0.7071,0,0.7071,

-0.1323,-0.1286,0.6547,0.6215,0.5704,-0.0002,-0.7072,0.707,
-0.1324,-0.1286,0.2646,0.6227,0.4727,0.3011,-0.9047,-0.3013,
-0.003,-0.1286,0.6547,0.6539,0.5708,0.5774,-0.5773,0.5773,

-0.1324,-0.1286,0.2646,0.594,0.7924,0.3011,-0.9047,-0.3013,
-0.1323,-0.0013,0.2646,0.6259,0.7918,0.7071,0,-0.7071,
-0.003,-0.1285,0.2646,0.5946,0.8248,0.5776,-0.5771,-0.5774,

-0.1324,-0.0013,0.0016,0.8895,0.0723,0.7071,0.7071,0,
-0.1324,-0.0013,0.1355,0.8896,0.1058,0.9044,0.3016,-0.3018,
-0.1324,-0.1286,0.1354,0.8577,0.1059,0.7071,-0.7071,-0.0002,

-0.1324,-0.1286,0.0016,0.8577,0.0724,0.7071,-0.7071,-0.0001,
-0.1324,-0.1286,-0.1319,0.8576,0.039,0.7071,-0.7071,0,
-0.1324,-0.0013,-0.132,0.8894,0.0389,0.9045,0.3016,0.3016,

-0.003,-0.0013,0.6547,0.628,0.9219,0.7071,0,0.7071,
-0.003,-0.0013,0.2646,0.6264,0.8242,0.7071,0,-0.7071,
-0.003,0.1283,0.2646,0.6589,0.8237,0.5774,0.5774,-0.5772,

-0.003,-0.0013,0.6547,0.628,0.9219,0.7071,0,0.7071,
-0.003,-0.1286,0.6547,0.5961,0.9224,0.5774,-0.5773,0.5773,
-0.003,-0.1285,0.2646,0.5946,0.8248,0.5776,-0.5771,-0.5774,

-0.1324,0.1284,0.2646,0.2757,0.6911,0.3014,0.9046,-0.3015,
-0.1324,0.1283,0.1355,0.2757,0.6588,0.5772,0.5774,-0.5775,
-0.26,0.1283,0.1355,0.3077,0.6588,-0.5774,0.5772,-0.5774,

-0.003,0.1283,0.2646,0.6589,0.8237,0.5774,0.5774,-0.5772,
-0.003,-0.0013,0.2646,0.6264,0.8242,0.7071,0,-0.7071,
-0.1323,-0.0013,0.2646,0.6259,0.7918,0.7071,0,-0.7071,

-0.1324,0.1283,0.1355,0.6578,0.759,0.5772,0.5774,-0.5775,
-0.1324,0.1284,0.2646,0.6583,0.7913,0.3014,0.9046,-0.3015,
-0.1323,-0.0013,0.2646,0.6259,0.7918,0.7071,0,-0.7071,

-0.1324,-0.0013,0.1355,0.6253,0.7595,0.9044,0.3016,-0.3018,
-0.2601,-0.0013,0.1355,0.6248,0.7276,-0.5778,0.5771,-0.5772,
-0.26,0.1283,0.1355,0.6572,0.727,-0.5774,0.5772,-0.5774,

-0.2601,-0.0013,0.1355,0.9216,0.1057,-0.5778,0.5771,-0.5772,
-0.1324,-0.0013,0.1355,0.8896,0.1058,0.9044,0.3016,-0.3018,
-0.1324,-0.0013,0.0016,0.8895,0.0723,0.7071,0.7071,0,

-0.1324,-0.0013,0.0016,0.8895,0.0723,0.7071,0.7071,0,
-0.1324,-0.0013,-0.132,0.8894,0.0389,0.9045,0.3016,0.3016,
-0.2601,-0.0013,-0.132,0.9214,0.0387,-0.5778,0.577,0.5773,

-0.1324,-0.0013,-0.132,0.8077,0.3617,0.9045,0.3016,0.3016,
-0.1324,-0.0013,-0.2631,0.8053,0.329,0.7073,0,0.7069,
-0.1324,0.1284,-0.2631,0.8377,0.3266,0.3015,0.9045,0.3015,

-0.1323,-0.0013,0.2646,0.6259,0.7918,0.7071,0,-0.7071,
-0.003,-0.0013,0.2646,0.6264,0.8242,0.7071,0,-0.7071,
-0.003,-0.1285,0.2646,0.5946,0.8248,0.5776,-0.5771,-0.5774,

-0.0033,-0.0013,-0.6525,0.7957,0.1995,0.7071,0,-0.7071,
-0.0033,0.1283,-0.6525,0.8281,0.1971,0.5772,0.5775,-0.5774,
-0.0032,0.1283,-0.2632,0.8353,0.2943,0.5775,0.5773,0.5772,

-0.0032,-0.1286,-0.2632,0.7712,0.2991,0.5776,-0.5773,0.5771,
-0.0033,-0.1285,-0.6525,0.7639,0.2019,0.5773,-0.5772,-0.5775,
-0.0033,-0.0013,-0.6525,0.7957,0.1995,0.7071,0,-0.7071,

-0.0033,-0.0013,-0.6525,0.7957,0.1995,0.7071,0,-0.7071,
-0.1324,-0.0013,-0.6525,0.7933,0.1673,-0.0002,0,-1,
-0.1321,0.1284,-0.6525,0.8256,0.1649,-0.0001,0.7072,-0.707,

-0.2601,-0.0013,-0.6524,0.7908,0.1354,-0.4476,0,-0.8943,
-0.2601,0.1283,-0.6524,0.8232,0.1329,-0.5775,0.5773,-0.5772,
-0.1321,0.1284,-0.6525,0.8256,0.1649,-0.0001,0.7072,-0.707,

-0.1324,-0.1286,-0.6525,0.7615,0.1697,-0.0001,-0.7073,-0.707,
-0.1324,-0.0013,-0.6525,0.7933,0.1673,-0.0002,0,-1,
-0.0033,-0.0013,-0.6525,0.7957,0.1995,0.7071,0,-0.7071,

-0.1324,-0.1286,-0.6525,0.7615,0.1697,-0.0001,-0.7073,-0.707,
-0.2601,-0.1285,-0.6524,0.7591,0.1378,-0.0007,-0.707,-0.7072,
-0.2601,-0.0013,-0.6524,0.7908,0.1354,-0.4476,0,-0.8943,

-0.1321,0.1284,-0.6525,0.8256,0.1649,-0.0001,0.7072,-0.707,
-0.1324,0.1284,-0.2631,0.9228,0.1573,0.3015,0.9045,0.3015,
-0.0032,0.1283,-0.2632,0.9253,0.1896,0.5775,0.5773,0.5772,

-0.1324,0.1284,-0.2631,0.9228,0.1573,0.3015,0.9045,0.3015,
-0.2601,0.1284,-0.2631,0.9204,0.1254,-0.7071,0.7071,0.0001,
-0.2601,0.1283,-0.1319,0.9531,0.1228,-0.5772,0.5771,0.5778,

-0.2601,0.1284,0.2646,0.3077,0.6911,-0.7071,0.7071,-0.0001,
-0.26,0.1283,0.6548,0.3077,0.7888,-0.5774,0.5772,0.5774,
-0.1323,0.1284,0.6547,0.2757,0.7888,0,0.7072,0.7071,

-0.1324,0.1284,0.2646,0.2757,0.6911,0.3014,0.9046,-0.3015,
-0.1323,0.1284,0.6547,0.2757,0.7888,0,0.7072,0.7071,
-0.003,0.1283,0.6547,0.2433,0.7888,0.5774,0.5773,0.5773,

-0.26,-0.0013,0.6547,0.629,0.9863,-0.4473,0,0.8944,
-0.1323,-0.0013,0.6547,0.6285,0.9543,0,0,1,
-0.1323,0.1284,0.6547,0.661,0.9538,0,0.7072,0.7071,

-0.1323,0.1284,0.6547,0.661,0.9538,0,0.7072,0.7071,
-0.1323,-0.0013,0.6547,0.6285,0.9543,0,0,1,
-0.003,-0.0013,0.6547,0.628,0.9219,0.7071,0,0.7071,

-0.003,-0.0013,0.6547,0.628,0.9219,0.7071,0,0.7071,
-0.1323,-0.0013,0.6547,0.6285,0.9543,0,0,1,
-0.1323,-0.1286,0.6547,0.5966,0.9548,-0.0002,-0.7072,0.707,

-0.1323,-0.1286,0.6547,0.5966,0.9548,-0.0002,-0.7072,0.707,
-0.1323,-0.0013,0.6547,0.6285,0.9543,0,0,1,
-0.26,-0.0013,0.6547,0.629,0.9863,-0.4473,0,0.8944,

-0.1324,-0.1286,0.2646,0.6227,0.4727,0.3011,-0.9047,-0.3013,
-0.26,-0.1285,0.2643,0.5907,0.4722,-0.0011,-1,0.0004,
-0.2601,-0.1286,0.1354,0.5912,0.44,-0.0003,-1,0.0001,

-0.2601,-0.1286,0.1354,0.9535,0.1056,-0.0003,-1,0.0001,
-0.2601,-0.1286,0.0018,0.9534,0.0722,0,-1,0,
-0.1324,-0.1286,0.0016,0.9853,0.072,0.7071,-0.7071,-0.0001,

-0.2601,-0.1286,-0.1318,0.9533,0.0387,-0.0001,-1,0.0001,
-0.1324,-0.1286,-0.1319,0.9852,0.0386,0.7071,-0.7071,0,
-0.1324,-0.1286,0.0016,0.9853,0.072,0.7071,-0.7071,-0.0001,

-0.1324,-0.1286,0.2646,0.6227,0.4727,0.3011,-0.9047,-0.3013,
-0.003,-0.1285,0.2646,0.6551,0.4731,0.5776,-0.5771,-0.5774,
-0.003,-0.1286,0.6547,0.6539,0.5708,0.5774,-0.5773,0.5773,

-0.2601,-0.1286,-0.1318,0.6291,0.1476,-0.0001,-1,0.0001,
-0.2601,-0.1286,-0.2631,0.6618,0.1452,-0.0003,-1,-0.0001,
-0.1324,-0.1286,-0.2631,0.6643,0.177,0.3015,-0.9046,0.3013,

-0.1324,-0.1286,-0.2631,0.6643,0.177,0.3015,-0.9046,0.3013,
-0.1324,-0.1286,-0.6525,0.7615,0.1697,-0.0001,-0.7073,-0.707,
-0.0033,-0.1285,-0.6525,0.7639,0.2019,0.5773,-0.5772,-0.5775,

-0.2601,-0.1285,-0.6524,0.7591,0.1378,-0.0007,-0.707,-0.7072,
-0.1324,-0.1286,-0.6525,0.7615,0.1697,-0.0001,-0.7073,-0.707,
-0.1324,-0.1286,-0.2631,0.6643,0.177,0.3015,-0.9046,0.3013,

-0.1324,-0.1286,-0.2631,0.7735,0.3313,0.3015,-0.9046,0.3013,
-0.0032,-0.1286,-0.2632,0.7712,0.2991,0.5776,-0.5773,0.5771,
-0.0032,-0.0013,-0.2631,0.8029,0.2967,0.7074,0,0.7068,

-0.0032,-0.0013,-0.2631,0.8029,0.2967,0.7074,0,0.7068,
-0.0032,0.1283,-0.2632,0.8353,0.2943,0.5775,0.5773,0.5772,
-0.1324,0.1284,-0.2631,0.8377,0.3266,0.3015,0.9045,0.3015,

-0.2601,0.1284,-0.2631,0.8158,0.0357,-0.7071,0.7071,0.0001,
-0.2601,-0.0013,-0.6524,0.7908,0.1354,-0.4476,0,-0.8943,
-0.2601,-0.0013,-0.132,0.7809,0.0055,-0.5778,0.577,0.5773,

-0.2601,0.1284,0.2646,0.5264,0.4715,-0.7071,0.7071,-0.0001,
-0.2601,-0.0013,0.1355,0.5593,0.4396,-0.5778,0.5771,-0.5772,
-0.26,-0.0013,0.6547,0.5577,0.5696,-0.4473,0,0.8944,

-0.2601,-0.1286,-0.2631,0.7516,0.0406,-1,0,0,
-0.2601,-0.0013,-0.6524,0.7908,0.1354,-1,0,0,
-0.2601,-0.1285,-0.6524,0.7591,0.1378,-1,0,0,

-0.2601,-0.1286,0.1354,0.5912,0.44,-1,-0.0001,0,
-0.26,-0.1285,0.2643,0.5907,0.4722,-1,-0.0001,0,
-0.2601,-0.0013,0.1355,0.5593,0.4396,-1,-0.0001,0,

-0.2601,-0.0013,0.1355,0.9216,0.1057,-1,-0.0001,0,
-0.2601,-0.0013,0.0018,0.9215,0.0723,-1,-0.0001,0,
-0.2601,-0.1286,0.1354,0.9535,0.1056,-1,-0.0001,0,

-0.26,-0.1285,0.2643,0.5907,0.4722,-1,0,0,
-0.26,-0.0013,0.6547,0.5577,0.5696,-1,0,0,
-0.2601,-0.0013,0.1355,0.5593,0.4396,-1,0,0,

-0.26,-0.1285,0.2643,0.5907,0.4722,-1,-0.0001,0,
-0.26,-0.1285,0.6547,0.5895,0.57,-1,-0.0001,0,
-0.26,-0.0013,0.6547,0.5577,0.5696,-1,-0.0001,0,

-0.2601,-0.0013,0.0018,0.9215,0.0723,-1,0,0.0001,
-0.2601,-0.1286,0.0018,0.9534,0.0722,-1,0,0.0001,
-0.2601,-0.1286,0.1354,0.9535,0.1056,-1,0,0.0001,

-0.2601,-0.1286,-0.2631,0.7516,0.0406,-1,0,0,
-0.2601,-0.0013,-0.132,0.7809,0.0055,-1,0,0,
-0.2601,-0.0013,-0.6524,0.7908,0.1354,-1,0,0,

-0.2601,-0.1286,-0.2631,0.7516,0.0406,-1,-0.0001,0.0001,
-0.2601,-0.1286,-0.1318,0.7491,0.0078,-1,-0.0001,0.0001,
-0.2601,-0.0013,-0.132,0.7809,0.0055,-1,-0.0001,0.0001,

-0.2601,-0.1286,-0.1318,0.9533,0.0387,-1,-0.0001,0,
-0.2601,-0.0013,0.0018,0.9215,0.0723,-1,-0.0001,0,
-0.2601,-0.0013,-0.132,0.9214,0.0387,-1,-0.0001,0,

-0.2601,-0.1286,-0.1318,0.9533,0.0387,-1,0,0,
-0.2601,-0.1286,0.0018,0.9534,0.0722,-1,0,0,
-0.2601,-0.0013,0.0018,0.9215,0.0723,-1,0,0,

-0.2601,-0.0013,-0.132,0.9854,0.1203,-0.5778,0.577,0.5773,
-0.1324,-0.0013,-0.132,0.9879,0.1522,0.9045,0.3016,0.3016,
-0.1324,0.1283,-0.132,0.9556,0.1547,0.5773,0.5772,0.5775,
		},
		96,
		{
			{
				vec3(-0.099   , 3.85,-19.574) ,
				vec3(-7.802   , 3.85,-19.572) ,
				vec3(-7.802, -3.855,  -19.573) ,
				vec3(-0.099  ,-3.855,-19.574) ,
				vec3(-7.801  , 3.85, 19.643) ,
				vec3(-0.089  ,3.85, 19.642) ,
				vec3(-0.089 ,-3.857, 19.642) ,
				vec3(-7.801  , -3.855,19.642)
			}
		},
		{ vec3(0,2.2f,0) }
	);//正视图正方形左侧
	ConfigModel(model5);

	Model model6
	(
		{ 0.0014,-0.1284,0.1341,0.6064,0.3672,-0.5774,-0.5773,0.5773,
0.0014,-0.0014,0.0041,0.5712,0.3384,-0.5775,0.5773,-0.5772,
0.0014,-0.1284,0.0041,0.6028,0.3348,-0.5775,-0.5772,-0.5773,

0.1307,-0.1285,-0.2616,0.0475,0.9624,-0.3015,-0.9046,0.3013,
0.1307,-0.0014,-0.1317,0.0094,0.9376,-0.9046,0.3014,0.3015,
0.1307,-0.0013,-0.2616,0.0412,0.9312,-0.7071,-0.0001,0.7071,

0.1307,-0.0014,-0.1317,0.0094,0.9376,-0.9046,0.3014,0.3015,
0.1307,-0.1285,-0.2616,0.0475,0.9624,-0.3015,-0.9046,0.3013,
0.1307,-0.1285,-0.1317,0.0156,0.9688,-0.7072,-0.707,0.0003,

0.1307,0.1283,0.1341,0.8963,0.2486,-0.5776,0.5771,-0.5774,
0.1309,-0.0013,0.1341,0.9145,0.2274,-0.7073,0.7069,-0.0018,
0.1307,-0.0013,0.2666,0.9379,0.255,-0.7066,-0.0006,-0.7076,

0.0014,-0.0013,-0.6513,0.1687,0.9057,-0.7074,-0.0003,-0.7068,
0.0014,0.1283,-0.2616,0.0666,0.893,-0.5774,0.5773,0.5774,
0.0014,0.1283,-0.6512,0.1623,0.8738,-0.5775,0.5775,-0.5771,

0.0014,-0.0013,0.6561,0.6273,0.103,-0.7072,-0.0004,0.707,
0.0014,-0.0009,0.2666,0.5393,0.061,-0.7071,0,-0.7072,
0.0014,-0.1284,0.2666,0.5531,0.0322,-0.5774,-0.5773,-0.5773,

0.1308,-0.1284,0.0041,0.2606,0.6116,-0.2097,-0.9503,0.23,
0.1307,-0.0014,0.004,0.2363,0.5876,-0.3017,0.9043,-0.3019,
0.1307,-0.0014,-0.1317,0.253,0.5638,-0.9046,0.3014,0.3015,

0.0014,-0.1284,-0.2616,0.0792,0.956,-0.5775,-0.5774,0.5772,
0.0014,-0.0013,-0.6513,0.1687,0.9057,-0.7074,-0.0003,-0.7068,
0.0016,-0.1284,-0.6513,0.1749,0.9369,-0.577,-0.5777,-0.5773,

0.0014,0.1283,0.2666,0.5254,0.0902,-0.5774,0.5773,-0.5773,
0.0014,-0.0013,0.6561,0.6273,0.103,-0.7072,-0.0004,0.707,
0.0014,0.1283,0.6561,0.6133,0.1323,-0.5774,0.5773,0.5773,

0.1307,-0.0013,-0.2616,0.0412,0.9312,-0.7071,-0.0001,0.7071,
0.1307,-0.0014,-0.1317,0.0094,0.9376,-0.9046,0.3014,0.3015,
0.1307,0.1283,-0.2616,0.0349,0.8994,-0.3923,0.9198,0.0039,

0.1307,-0.0013,0.2666,0.9379,0.255,-0.7066,-0.0006,-0.7076,
0.1309,-0.0013,0.1341,0.9145,0.2274,-0.7073,0.7069,-0.0018,
0.1309,-0.1285,0.1341,0.9399,0.2085,-0.3015,-0.9048,0.3009,

0.1307,-0.0013,0.2666,0.9379,0.255,-0.7066,-0.0006,-0.7076,
0.1309,-0.1285,0.1341,0.9399,0.2085,-0.3015,-0.9048,0.3009,
0.1309,-0.1285,0.2666,0.9618,0.2334,-0.388,-0.8696,-0.3055,

0.1307,-0.0013,0.6561,0.9318,0.4036,0.0001,0,1,
0.1307,-0.1285,0.656,0.939,0.4346,0.0002,-0.7072,0.707,
0.2586,-0.1284,0.656,0.9078,0.4418,0.0003,-0.7071,0.7071,

0.2586,-0.1284,0.656,0.5182,0.983,0.0003,-0.7071,0.7071,
0.1309,-0.1285,0.2666,0.4804,0.8876,-0.388,-0.8696,-0.3055,
0.2586,-0.1285,0.2666,0.5123,0.8856,0.0001,-1,0.0001,

0.1307,-0.1285,0.656,0.4863,0.9849,0.0002,-0.7072,0.707,
0.1309,-0.1285,0.2666,0.4804,0.8876,-0.388,-0.8696,-0.3055,
0.2586,-0.1284,0.656,0.5182,0.983,0.0003,-0.7071,0.7071,

0.1309,-0.1285,0.1341,0.9399,0.2085,-0.3015,-0.9048,0.3009,
0.2585,-0.1285,0.1341,0.9638,0.1873,0,-1,-0.0001,
0.2586,-0.1285,0.2666,0.9858,0.2121,0.0001,-1,0.0001,

0.2585,-0.1285,0.1341,0.2976,0.6413,0,-1,-0.0001,
0.1308,-0.1284,0.0041,0.2606,0.6116,-0.2097,-0.9503,0.23,
0.2585,-0.1285,0.003,0.2938,0.6136,-0.0004,-1,0,

0.2585,-0.1285,0.1341,0.2976,0.6413,0,-1,-0.0001,
0.1309,-0.1285,0.1341,0.2708,0.6449,-0.3015,-0.9048,0.3009,
0.1308,-0.1284,0.0041,0.2606,0.6116,-0.2097,-0.9503,0.23,

0.1308,-0.1284,0.0041,0.2606,0.6116,-0.2097,-0.9503,0.23,
0.2585,-0.1285,-0.1317,0.3015,0.5858,0,-1,0.0001,
0.2585,-0.1285,0.003,0.2938,0.6136,-0.0004,-1,0,

0.1307,-0.1285,-0.1317,0.3718,0.9308,-0.7072,-0.707,0.0003,
0.1307,-0.1285,-0.2616,0.3399,0.937,-0.3015,-0.9046,0.3013,
0.2585,-0.1285,-0.1317,0.3656,0.8994,0,-1,0.0001,

0.1307,-0.1285,-0.2616,0.3399,0.937,-0.3015,-0.9046,0.3013,
0.2585,-0.1284,-0.6513,0.2379,0.9244,0.0002,-0.7071,-0.7072,
0.2585,-0.1285,-0.2616,0.3337,0.9056,0.0002,-1,-0.0001,

0.1307,-0.0013,-0.6514,0.2004,0.8994,-0.0003,0,-1,
0.2585,-0.0013,-0.6513,0.2318,0.8932,0.4476,0.0009,-0.8942,
0.2585,-0.1284,-0.6513,0.2379,0.9244,0.0002,-0.7071,-0.7072,

0.1307,-0.0013,-0.6514,0.2004,0.8994,-0.0003,0,-1,
0.2583,0.1283,-0.6513,0.2255,0.8614,0.5768,0.5778,-0.5774,
0.2585,-0.0013,-0.6513,0.2318,0.8932,0.4476,0.0009,-0.8942,

0.2583,0.1283,-0.6513,0.2255,0.8614,0.5768,0.5778,-0.5774,
0.1307,0.1283,-0.2616,0.3151,0.8112,-0.3923,0.9198,0.0039,
0.2583,0.1283,-0.2616,0.3213,0.8425,0.7064,0.7078,0,

0.1307,0.1283,-0.6513,0.2194,0.83,-0.0001,0.7073,-0.7069,
0.1307,0.1283,-0.2616,0.3151,0.8112,-0.3923,0.9198,0.0039,
0.2583,0.1283,-0.6513,0.2255,0.8614,0.5768,0.5778,-0.5774,

0.1307,0.1283,-0.2616,0.3151,0.8112,-0.3923,0.9198,0.0039,
0.2583,0.1283,-0.1317,0.3532,0.8363,0.5768,0.5778,0.5775,
0.2583,0.1283,-0.2616,0.3213,0.8425,0.7064,0.7078,0,

0.1307,0.1283,-0.1317,0.5317,0.276,-0.5774,0.5773,0.5774,
0.1307,-0.0014,-0.1317,0.5353,0.3082,-0.9046,0.3014,0.3015,
0.2585,-0.0013,-0.1317,0.5035,0.3117,0.577,0.5783,0.5768,

0.1307,-0.0014,-0.1317,0.5353,0.3082,-0.9046,0.3014,0.3015,
0.2585,-0.0014,0.004,0.5072,0.3455,-0.0001,1,0,
0.2585,-0.0013,-0.1317,0.5035,0.3117,0.577,0.5783,0.5768,

0.2585,-0.0014,0.004,0.5072,0.3455,-0.0001,1,0,
0.1309,-0.0013,0.1341,0.5426,0.3744,-0.7073,0.7069,-0.0018,
0.2586,-0.0013,0.1341,0.5108,0.3779,0.5774,0.5778,-0.5769,

0.2585,-0.0014,0.004,0.5072,0.3455,-0.0001,1,0,
0.1307,-0.0014,0.004,0.539,0.342,-0.3017,0.9043,-0.3019,
0.1309,-0.0013,0.1341,0.5426,0.3744,-0.7073,0.7069,-0.0018,

0.1309,-0.0013,0.1341,0.9145,0.2274,-0.7073,0.7069,-0.0018,
0.2585,0.1283,0.1341,0.8694,0.2443,0.5779,0.5773,-0.5768,
0.2586,-0.0013,0.1341,0.8843,0.2144,0.5774,0.5778,-0.5769,

0.1307,0.1283,0.1341,0.8963,0.2486,-0.5776,0.5771,-0.5774,
0.1307,0.1283,0.2666,0.9068,0.2765,-0.3015,0.9045,-0.3015,
0.2583,0.1283,0.2666,0.8726,0.2832,0.7064,0.7078,0.0003,

0.1307,0.1283,0.2666,0.9068,0.2765,-0.3015,0.9045,-0.3015,
0.1307,0.1283,0.6561,0.9246,0.372,0.0001,0.7072,0.707,
0.2583,0.1283,0.2666,0.8726,0.2832,0.7064,0.7078,0.0003,

0.1307,-0.0013,0.6561,0.9318,0.4036,0.0001,0,1,
0.2586,-0.0013,0.656,0.9006,0.4107,0.4476,0.0008,0.8942,
0.2583,0.1283,0.656,0.8934,0.3789,0.5768,0.5779,0.5774,

0.2583,0.1283,0.2666,0.5765,0.8818,0.7064,0.7078,0.0003,
0.2583,0.1283,0.656,0.5824,0.9791,0.5768,0.5779,0.5774,
0.2586,-0.0013,0.656,0.55,0.9811,0.4476,0.0008,0.8942,

0.2586,-0.0013,0.1341,0.5421,0.8506,0.5774,0.5778,-0.5769,
0.2585,0.1283,0.1341,0.5745,0.8487,0.5779,0.5773,-0.5768,
0.2583,0.1283,0.2666,0.5765,0.8818,0.7064,0.7078,0.0003,

0.2585,-0.0013,-0.6513,0.2318,0.8932,0.4476,0.0009,-0.8942,
0.2583,0.1283,-0.2616,0.3213,0.8425,0.7064,0.7078,0,
0.2585,-0.0013,-0.1317,0.3595,0.8681,0.577,0.5783,0.5768,

0.2585,-0.0013,-0.6513,0.2318,0.8932,0.4476,0.0009,-0.8942,
0.2583,0.1283,-0.6513,0.2255,0.8614,0.5768,0.5778,-0.5774,
0.2583,0.1283,-0.2616,0.3213,0.8425,0.7064,0.7078,0,

0.0014,-0.1284,0.1341,0.5784,0.4024,-0.5774,-0.5773,0.5773,
0.1309,-0.1285,0.1341,0.5461,0.406,-0.3015,-0.9048,0.3009,
0.1309,-0.0013,0.1341,0.5426,0.3744,-0.7073,0.7069,-0.0018,

0.0014,-0.1284,0.0041,0.2302,0.6294,-0.5775,-0.5772,-0.5773,
0.1308,-0.1284,0.0041,0.2606,0.6116,-0.2097,-0.9503,0.23,
0.1309,-0.1285,0.1341,0.2708,0.6449,-0.3015,-0.9048,0.3009,

0.1307,-0.1284,0.004,0.2577,0.6115,0.059,-0.2173,-0.9743,
0.0014,-0.1284,0.0041,0.2302,0.6294,-0.5775,-0.5772,-0.5773,
0.1307,-0.0014,0.004,0.2363,0.5876,-0.3017,0.9043,-0.3019,

0.1307,-0.0014,0.004,0.2363,0.5876,-0.3017,0.9043,-0.3019,
0.1308,-0.1284,0.0041,0.2606,0.6116,-0.2097,-0.9503,0.23,
0.1307,-0.1284,0.004,0.2577,0.6115,0.059,-0.2173,-0.9743,

0.0014,-0.0014,0.0041,0.5712,0.3384,-0.5775,0.5773,-0.5772,
0.1309,-0.0013,0.1341,0.5426,0.3744,-0.7073,0.7069,-0.0018,
0.1307,-0.0014,0.004,0.539,0.342,-0.3017,0.9043,-0.3019,

0.0014,-0.0013,-0.2616,0.073,0.9248,-0.7071,0,0.7071,
0.1307,-0.1285,-0.2616,0.0475,0.9624,-0.3015,-0.9046,0.3013,
0.1307,-0.0013,-0.2616,0.0412,0.9312,-0.7071,-0.0001,0.7071,

0.0014,-0.1284,-0.2616,0.3461,0.9688,-0.5775,-0.5774,0.5772,
0.0016,-0.1284,-0.6513,0.2503,0.9875,-0.577,-0.5777,-0.5773,
0.1307,-0.1285,-0.2616,0.3399,0.937,-0.3015,-0.9046,0.3013,

0.0016,-0.1284,-0.6513,0.1749,0.9369,-0.577,-0.5777,-0.5773,
0.1307,-0.0013,-0.6514,0.2004,0.8994,-0.0003,0,-1,
0.1307,-0.1285,-0.6513,0.2066,0.9306,-0.0002,-0.7074,-0.7068,

0.0014,0.1283,-0.6512,0.1623,0.8738,-0.5775,0.5775,-0.5771,
0.1307,0.1283,-0.6513,0.1941,0.8675,-0.0001,0.7073,-0.7069,
0.1307,-0.0013,-0.6514,0.2004,0.8994,-0.0003,0,-1,

0.0014,0.1283,-0.6512,0.2131,0.7982,-0.5775,0.5775,-0.5771,
0.1307,0.1283,-0.2616,0.3151,0.8112,-0.3923,0.9198,0.0039,
0.1307,0.1283,-0.6513,0.2194,0.83,-0.0001,0.7073,-0.7069,

0.0014,0.1283,-0.2616,0.0666,0.893,-0.5774,0.5773,0.5774,
0.0014,-0.0013,-0.2616,0.073,0.9248,-0.7071,0,0.7071,
0.1307,0.1283,-0.2616,0.0349,0.8994,-0.3923,0.9198,0.0039,

0.0014,-0.0009,0.2666,0.5393,0.061,-0.7071,0,-0.7072,
0.1307,0.1283,0.2666,0.4962,0.0763,-0.3015,0.9045,-0.3015,
0.1307,-0.0013,0.2666,0.5101,0.047,-0.7066,-0.0006,-0.7076,

0.0014,0.1283,0.2666,0.5254,0.0902,-0.5774,0.5773,-0.5773,
0.0014,0.1283,0.6561,0.4839,0.1784,-0.5774,0.5773,0.5773,
0.1307,0.1283,0.6561,0.4546,0.1646,0.0001,0.7072,0.707,

0.0014,0.1283,0.6561,0.4839,0.1784,-0.5774,0.5773,0.5773,
0.1307,-0.0013,0.6561,0.4408,0.194,0.0001,0,1,
0.1307,0.1283,0.6561,0.4546,0.1646,0.0001,0.7072,0.707,

0.0016,-0.1284,0.6561,0.4566,0.2365,-0.5768,-0.5777,0.5776,
0.1307,-0.1285,0.656,0.4273,0.2228,0.0002,-0.7072,0.707,
0.1307,-0.0013,0.6561,0.4408,0.194,0.0001,0,1,

0.0016,-0.1284,0.6561,0.454,0.9869,-0.5768,-0.5777,0.5776,
0.1309,-0.1285,0.2666,0.4804,0.8876,-0.388,-0.8696,-0.3055,
0.1307,-0.1285,0.656,0.4863,0.9849,0.0002,-0.7072,0.707,

0.0014,-0.1284,0.2666,0.5531,0.0322,-0.5774,-0.5773,-0.5773,
0.1307,-0.0013,0.2666,0.5101,0.047,-0.7066,-0.0006,-0.7076,
0.1309,-0.1285,0.2666,0.5238,0.0182,-0.388,-0.8696,-0.3055,

0.1309,-0.1284,0.0041,0.2584,0.6119,-0.0002,-0.9916,-0.1296,
0.1308,-0.1284,0.0041,0.2606,0.6116,-0.2097,-0.9503,0.23,
0.0014,-0.1284,0.0041,0.2302,0.6294,-0.5775,-0.5772,-0.5773,

0.1307,0.1283,-0.2616,0.9525,0.9877,-0.3923,0.9198,0.0039,
0.1307,0.1283,-0.2616,0.9525,0.9877,-0.0001,0.6868,-0.7268,
0.0014,0.1283,-0.2616,0.9849,0.9878,-0.5774,0.5773,0.5774,

0.1309,-0.1285,0.2666,0.5238,0.0182,-0.388,-0.8696,-0.3055,
0.1308,-0.1284,0.2666,0.5239,0.0182,-0.0001,-0.5257,0.8507,
0.0014,-0.1284,0.2666,0.5531,0.0322,-0.5774,-0.5773,-0.5773,

0.1307,-0.0013,0.2666,0.9379,0.255,-0.7066,-0.0006,-0.7076,
0.1307,0.1283,0.2666,0.9068,0.2765,-0.3015,0.9045,-0.3015,
0.1307,0.1283,0.1341,0.8963,0.2486,-0.5776,0.5771,-0.5774,

0.0014,-0.0014,0.0041,0.5712,0.3384,-0.5775,0.5773,-0.5772,
0.0014,-0.0013,0.1341,0.5748,0.3707,-0.5774,0.5774,0.5772,
0.1309,-0.0013,0.1341,0.5426,0.3744,-0.7073,0.7069,-0.0018,

0.0014,-0.1284,0.0041,0.2302,0.6294,-0.5775,-0.5772,-0.5773,
0.0014,-0.0014,0.0041,0.2107,0.6056,-0.5775,0.5773,-0.5772,
0.1307,-0.0014,0.004,0.2363,0.5876,-0.3017,0.9043,-0.3019,

0.0014,-0.1284,0.1341,0.6064,0.3672,-0.5774,-0.5773,0.5773,
0.0014,-0.0013,0.1341,0.5748,0.3707,-0.5774,0.5774,0.5772,
0.0014,-0.0014,0.0041,0.5712,0.3384,-0.5775,0.5773,-0.5772,

0.1309,-0.0013,0.1341,0.5426,0.3744,-0.7073,0.7069,-0.0018,
0.0014,-0.0013,0.1341,0.5748,0.3707,-0.5774,0.5774,0.5772,
0.0014,-0.1284,0.1341,0.5784,0.4024,-0.5774,-0.5773,0.5773,

0.1309,-0.1285,0.1341,0.2708,0.6449,-0.3015,-0.9048,0.3009,
0.0014,-0.1284,0.1341,0.2424,0.6587,-0.5774,-0.5773,0.5773,
0.0014,-0.1284,0.0041,0.2302,0.6294,-0.5775,-0.5772,-0.5773,

0.1307,-0.0014,-0.1317,0.253,0.5638,-0.9046,0.3014,0.3015,
0.1307,-0.1285,-0.1317,0.2751,0.5794,-0.7072,-0.707,0.0003,
0.1308,-0.1284,0.0041,0.2606,0.6116,-0.2097,-0.9503,0.23,

0.1309,-0.1284,0.0041,0.2584,0.6119,-0.0002,-0.9916,-0.1296,
0.0014,-0.1284,0.0041,0.2302,0.6294,-0.5775,-0.5772,-0.5773,
0.1307,-0.1284,0.004,0.2577,0.6115,0.059,-0.2173,-0.9743,

0.1307,0.1283,-0.2616,0.3151,0.8112,-0.3923,0.9198,0.0039,
0.1307,0.1283,-0.1317,0.347,0.8049,-0.5774,0.5773,0.5774,
0.2583,0.1283,-0.1317,0.3532,0.8363,0.5768,0.5778,0.5775,

0.1307,-0.0014,-0.1317,0.0094,0.9376,-0.9046,0.3014,0.3015,
0.1307,0.1283,-0.1317,0.003,0.9058,-0.5774,0.5773,0.5774,
0.1307,0.1283,-0.2616,0.0349,0.8994,-0.3923,0.9198,0.0039,

0.0014,-0.0013,-0.2616,0.073,0.9248,-0.7071,0,0.7071,
0.1307,-0.0013,-0.2616,0.0412,0.9312,-0.7071,-0.0001,0.7071,
0.1307,0.1283,-0.2616,0.0349,0.8994,-0.3923,0.9198,0.0039,

0.0014,-0.0013,-0.2616,0.073,0.9248,-0.7071,0,0.7071,
0.0014,-0.1284,-0.2616,0.0792,0.956,-0.5775,-0.5774,0.5772,
0.1307,-0.1285,-0.2616,0.0475,0.9624,-0.3015,-0.9046,0.3013,

0.0014,-0.1284,-0.2616,0.0792,0.956,-0.5775,-0.5774,0.5772,
0.0014,-0.0013,-0.2616,0.073,0.9248,-0.7071,0,0.7071,
0.0014,-0.0013,-0.6513,0.1687,0.9057,-0.7074,-0.0003,-0.7068,

0.0014,-0.0013,-0.6513,0.1687,0.9057,-0.7074,-0.0003,-0.7068,
0.0014,-0.0013,-0.2616,0.073,0.9248,-0.7071,0,0.7071,
0.0014,0.1283,-0.2616,0.0666,0.893,-0.5774,0.5773,0.5774,

0.1307,-0.0013,-0.6514,0.2004,0.8994,-0.0003,0,-1,
0.0014,-0.0013,-0.6513,0.1687,0.9057,-0.7074,-0.0003,-0.7068,
0.0014,0.1283,-0.6512,0.1623,0.8738,-0.5775,0.5775,-0.5771,

0.0016,-0.1284,-0.6513,0.1749,0.9369,-0.577,-0.5777,-0.5773,
0.0014,-0.0013,-0.6513,0.1687,0.9057,-0.7074,-0.0003,-0.7068,
0.1307,-0.0013,-0.6514,0.2004,0.8994,-0.0003,0,-1,

0.1307,-0.0013,-0.6514,0.2004,0.8994,-0.0003,0,-1,
0.1307,0.1283,-0.6513,0.1941,0.8675,-0.0001,0.7073,-0.7069,
0.2583,0.1283,-0.6513,0.2255,0.8614,0.5768,0.5778,-0.5774,

0.2585,-0.1284,-0.6513,0.2379,0.9244,0.0002,-0.7071,-0.7072,
0.1307,-0.1285,-0.6513,0.2066,0.9306,-0.0002,-0.7074,-0.7068,
0.1307,-0.0013,-0.6514,0.2004,0.8994,-0.0003,0,-1,

0.0014,0.1283,-0.6512,0.9852,0.8902,-0.5775,0.5775,-0.5771,
0.0014,0.1283,-0.2616,0.9849,0.9878,-0.5774,0.5773,0.5774,
0.1307,0.1283,-0.2616,0.9525,0.9877,-0.0001,0.6868,-0.7268,

0.0014,-0.0009,0.2666,0.5393,0.061,-0.7071,0,-0.7072,
0.0014,0.1283,0.2666,0.5254,0.0902,-0.5774,0.5773,-0.5773,
0.1307,0.1283,0.2666,0.4962,0.0763,-0.3015,0.9045,-0.3015,

0.0014,-0.1284,0.2666,0.5531,0.0322,-0.5774,-0.5773,-0.5773,
0.0014,-0.0009,0.2666,0.5393,0.061,-0.7071,0,-0.7072,
0.1307,-0.0013,0.2666,0.5101,0.047,-0.7066,-0.0006,-0.7076,

0.0014,-0.1284,0.2666,0.5531,0.0322,-0.5774,-0.5773,-0.5773,
0.0016,-0.1284,0.6561,0.6411,0.0743,-0.5768,-0.5777,0.5776,
0.0014,-0.0013,0.6561,0.6273,0.103,-0.7072,-0.0004,0.707,

0.0014,0.1283,0.2666,0.5254,0.0902,-0.5774,0.5773,-0.5773,
0.0014,-0.0009,0.2666,0.5393,0.061,-0.7071,0,-0.7072,
0.0014,-0.0013,0.6561,0.6273,0.103,-0.7072,-0.0004,0.707,

0.2583,0.1283,0.2666,0.8726,0.2832,0.7064,0.7078,0.0003,
0.2585,0.1283,0.1341,0.8694,0.2443,0.5779,0.5773,-0.5768,
0.1307,0.1283,0.1341,0.8963,0.2486,-0.5776,0.5771,-0.5774,

0.1307,0.1283,0.6561,0.9246,0.372,0.0001,0.7072,0.707,
0.2583,0.1283,0.656,0.8934,0.3789,0.5768,0.5779,0.5774,
0.2583,0.1283,0.2666,0.8726,0.2832,0.7064,0.7078,0.0003,

0.1307,0.1283,0.6561,0.4546,0.1646,0.0001,0.7072,0.707,
0.1307,0.1283,0.2666,0.4962,0.0763,-0.3015,0.9045,-0.3015,
0.0014,0.1283,0.2666,0.5254,0.0902,-0.5774,0.5773,-0.5773,

0.1307,-0.0014,-0.1317,0.5353,0.3082,-0.9046,0.3014,0.3015,
0.1307,-0.0014,0.004,0.539,0.342,-0.3017,0.9043,-0.3019,
0.2585,-0.0014,0.004,0.5072,0.3455,-0.0001,1,0,

0.0014,0.1283,0.6561,0.4839,0.1784,-0.5774,0.5773,0.5773,
0.0014,-0.0013,0.6561,0.4701,0.2077,-0.7072,-0.0004,0.707,
0.1307,-0.0013,0.6561,0.4408,0.194,0.0001,0,1,

0.2583,0.1283,0.656,0.8934,0.3789,0.5768,0.5779,0.5774,
0.1307,0.1283,0.6561,0.9246,0.372,0.0001,0.7072,0.707,
0.1307,-0.0013,0.6561,0.9318,0.4036,0.0001,0,1,

0.1307,-0.0013,0.6561,0.4408,0.194,0.0001,0,1,
0.0014,-0.0013,0.6561,0.4701,0.2077,-0.7072,-0.0004,0.707,
0.0016,-0.1284,0.6561,0.4566,0.2365,-0.5768,-0.5777,0.5776,

0.2586,-0.1284,0.656,0.9078,0.4418,0.0003,-0.7071,0.7071,
0.2586,-0.0013,0.656,0.9006,0.4107,0.4476,0.0008,0.8942,
0.1307,-0.0013,0.6561,0.9318,0.4036,0.0001,0,1,

0.2586,-0.0013,0.656,0.55,0.9811,0.4476,0.0008,0.8942,
0.2586,-0.0013,0.1341,0.5421,0.8506,0.5774,0.5778,-0.5769,
0.2583,0.1283,0.2666,0.5765,0.8818,0.7064,0.7078,0.0003,

0.2585,-0.0013,-0.1317,0.5035,0.3117,0.577,0.5783,0.5768,
0.2583,0.1283,-0.1317,0.5,0.2795,0.5768,0.5778,0.5775,
0.1307,0.1283,-0.1317,0.5317,0.276,-0.5774,0.5773,0.5774,

0.2583,0.1283,-0.2616,0.3213,0.8425,0.7064,0.7078,0,
0.2583,0.1283,-0.1317,0.3532,0.8363,0.5768,0.5778,0.5775,
0.2585,-0.0013,-0.1317,0.3595,0.8681,0.577,0.5783,0.5768,

0.2585,-0.1285,-0.1317,0.3656,0.8994,1,0,-0.0001,
0.2585,-0.1285,-0.2616,0.3337,0.9056,1,0,-0.0001,
0.2585,-0.0013,-0.1317,0.3595,0.8681,1,0,-0.0001,

0.2586,-0.0013,0.656,0.55,0.9811,1,-0.0001,0,
0.2586,-0.1284,0.656,0.5182,0.983,1,-0.0001,0,
0.2586,-0.1285,0.2666,0.5123,0.8856,1,-0.0001,0,

0.2586,-0.0013,0.1341,0.5108,0.3779,1,0,-0.0001,
0.2585,-0.1285,0.1341,0.4791,0.3814,1,0,-0.0001,
0.2585,-0.1285,0.003,0.4755,0.3488,1,0,-0.0001,

0.2585,-0.0013,-0.1317,0.5035,0.3117,1,-0.0001,0,
0.2585,-0.0014,0.004,0.5072,0.3455,1,-0.0001,0,
0.2585,-0.1285,0.003,0.4755,0.3488,1,-0.0001,0,

0.2586,-0.1285,0.2666,0.9858,0.2121,1,0,0,
0.2585,-0.1285,0.1341,0.9638,0.1873,1,0,0,
0.2586,-0.0013,0.1341,0.9876,0.1662,1,0,0,

0.2586,-0.1285,0.2666,0.5123,0.8856,1,0,0,
0.2586,-0.0013,0.1341,0.5421,0.8506,1,0,0,
0.2586,-0.0013,0.656,0.55,0.9811,1,0,0,

0.2585,-0.1285,0.003,0.4755,0.3488,1,-0.0001,0,
0.2585,-0.0014,0.004,0.5072,0.3455,1,-0.0001,0,
0.2586,-0.0013,0.1341,0.5108,0.3779,1,-0.0001,0,

0.2585,-0.1285,0.003,0.4755,0.3488,1,0,0,
0.2585,-0.1285,-0.1317,0.4718,0.3152,1,0,0,
0.2585,-0.0013,-0.1317,0.5035,0.3117,1,0,0,

0.2585,-0.1285,-0.2616,0.3337,0.9056,1,0,0,
0.2585,-0.0013,-0.6513,0.2318,0.8932,1,0,0,
0.2585,-0.0013,-0.1317,0.3595,0.8681,1,0,0,

0.2585,-0.1285,-0.2616,0.3337,0.9056,1,0,0,
0.2585,-0.1284,-0.6513,0.2379,0.9244,1,0,0,
0.2585,-0.0013,-0.6513,0.2318,0.8932,1,0,0,

0.2586,-0.1285,0.2666,0.9858,0.2121,0.0001,-1,0.0001,
0.1309,-0.1285,0.2666,0.9618,0.2334,-0.388,-0.8696,-0.3055,
0.1309,-0.1285,0.1341,0.9399,0.2085,-0.3015,-0.9048,0.3009,

0.1308,-0.1284,0.0041,0.2606,0.6116,-0.2097,-0.9503,0.23,
0.1307,-0.1285,-0.1317,0.2751,0.5794,-0.7072,-0.707,0.0003,
0.2585,-0.1285,-0.1317,0.3015,0.5858,0,-1,0.0001,

0.1307,-0.1285,-0.2616,0.3399,0.937,-0.3015,-0.9046,0.3013,
0.2585,-0.1285,-0.2616,0.3337,0.9056,0.0002,-1,-0.0001,
0.2585,-0.1285,-0.1317,0.3656,0.8994,0,-1,0.0001,

0.0016,-0.1284,0.6561,0.6411,0.0743,-0.5768,-0.5777,0.5776,
0.0014,-0.1284,0.2666,0.5531,0.0322,-0.5774,-0.5773,-0.5773,
0.1308,-0.1284,0.2666,0.5671,0.003,-0.0001,-0.5257,0.8507,

0.1307,-0.1285,-0.2616,0.3399,0.937,-0.3015,-0.9046,0.3013,
0.1307,-0.1285,-0.6513,0.2441,0.9558,-0.0002,-0.7074,-0.7068,
0.2585,-0.1284,-0.6513,0.2379,0.9244,0.0002,-0.7071,-0.7072,

0.0016,-0.1284,-0.6513,0.2503,0.9875,-0.577,-0.5777,-0.5773,
0.1307,-0.1285,-0.6513,0.2441,0.9558,-0.0002,-0.7074,-0.7068,
0.1307,-0.1285,-0.2616,0.3399,0.937,-0.3015,-0.9046,0.3013,

0.1309,-0.0013,0.1341,0.9145,0.2274,-0.7073,0.7069,-0.0018,
0.1307,0.1283,0.1341,0.8963,0.2486,-0.5776,0.5771,-0.5774,
0.2585,0.1283,0.1341,0.8694,0.2443,0.5779,0.5773,-0.5768,
		},
		109,
		{
			{
				vec3(0.042 ,3.848, 19.682) ,
				vec3(7.75  ,  3.848,19.681) ,
				vec3(7.757 , -3.853, 19.681) ,
				vec3(0.047  , -3.854,19.682) ,
				vec3(7.749 , 3.848,  -19.539) ,
				vec3(0.042 , 3.848,  -19.537) ,
				vec3(0.047  ,  -3.853, -19.538) ,
				vec3(7.756 , -3.853, -19.54)
			}
		},
		{ vec3(0,0.27f,0),vec3(1.1f,0.4f,0) }
	);
	ConfigModel(model6);//正视图正方形右侧

	Model background
	(
		{
		-10,5,-5,0,1,//左上
		10,5,-5,1,1,//右上
		-10,-5,-5,0,0,//左下

		10,-5,-5,1,0,//右下
		10,5,-5,1,1,//右上
		-10,-5,-5,0,0,//左下
		},
		2
		);
	background.ConfigAttribute(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	background.ConfigAttribute(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	//初始化模型结束

	InitModelTexture(modelShader, "Resources/Textures/Diffuse/05.png", "Resources/Textures/Specular/05.png");
	InitModelTexture(backGroundShader, backgroundPath);

	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);//当模板或深度测试不通过则保留当前模板值，否则模板值会设定为1

	InitModels({ &model1,&model2, &model3, &model4, &model5, &model6 });

	InitAnimationSequence({ &model1,&model2, &model3, &model4, &model5, &model6 }, { 2,4,3,5,0,1 });

	InitRecordedModels();

	/*………………………………渲染循环………………………………*/
	while (!glfwWindowShouldClose(mainWindow.window))//当窗口没有关闭
	{
		ClearAllBuffer();

		GetDeltaTime();

		SetProjectionMatrix(mainWindow, isOrtho, fov);

		{
			ActiveandBindBackgroundTexture();
			RenderBackground(backGroundShader, background);
		}

		if (!isLockLightPoiiton)//如果不锁定灯光位置
		{
			lightPosition = vec3((lastmousePosition.x - (1920.0f / 2)) / (1920.0f / 2), (lastmousePosition.y - (1080.0f / 2)) / (-1080.0f / 2), lightDepth);//使灯光随鼠标进行移动
		}
		GenSpotLight(modelShader, lightPosition, vec3(0, 0, -1), innerCutoffAngle, outerCutoffAngle);
		SetLightProperties(modelShader, vec4(ambient.x, ambient.y, ambient.z, ambient.w), vec4(diffuse.x, diffuse.y, diffuse.z, diffuse.w), vec4(specular.x, specular.y, specular.z, specular.w), pow(2, shininess), 1.0f, 0.0014f, 0.000007f);

		//判断是否有模型被聚焦
		{
			isAnyModelFocused = false;//假定没有模型被聚焦
			if (focusedModelIndex != -1)//如果存在聚焦模型索引
			{
				isAnyModelFocused = true;
			}
		}

		if (!isAnyModelFocused)//如果没有物体被聚焦
		{
			if (isRecordedModelMartix)
			{
				for (size_t i = 0; i < models.size(); i++)
				{
					models[i]->model = modelsforRecord[i];//还原所有模型的模型矩阵
				}
				translateSum = translateSumforRecord;
				isRecordedModelMartix = false;
			}

			PlayAnimation(animationTime, modelShader);

			if (!ImGui::IsAnyItemActive() && !ImGui::IsAnyWindowHovered())//除去鼠标在ImGui上的情况
			{
				if (isTranslate)//如果进行位移
				{
					translateSum += vec3(((mousePositionDelta * vec2(isTranslate)) + vec2(translateKeyDelta.w, translateKeyDelta.x) - vec2(translateKeyDelta.z, translateKeyDelta.y)) * modelTranslateSensitivity, 0);//记录总位移量
				}
				SetAllModelsTransform
				(
					vec3(((mousePositionDelta * vec2(isTranslate)) + vec2(translateKeyDelta.w, translateKeyDelta.x) - vec2(translateKeyDelta.z, translateKeyDelta.y)) * modelTranslateSensitivity, 0),
					((mousePositionDelta * vec2(isRotate)) + vec2(rotateKeyDelta.w, rotateKeyDelta.x) - vec2(rotateKeyDelta.z, rotateKeyDelta.y)) * modelRotateSensitivity,
					vec3(1 + (mouseScrollDelta + scaleKeyDelta.y - scaleKeyDelta.x) * modelScaleSensitivity)
				);
				ResetMouseDelta();
				ResetKeyDelta();
			}
		}
		else//如果有任意物体被聚焦
		{
			if (!isRecordedModelMartix)
			{
				for (size_t i = 0; i < models.size(); i++)
				{
					modelsforRecord[i] = models[i]->model;//记录所有模型的模型矩阵
				}
				translateSumforRecord = translateSum;//记录物体位移总量
				models[focusedModelIndex]->ResetTransform();
				isRecordedModelMartix = true;
			}
			if (!ImGui::IsAnyItemActive() && !ImGui::IsAnyWindowHovered())//除去鼠标在ImGui上的情况
			{
				if (isTranslate)//如果进行位移
				{
					translateSum += vec3(((mousePositionDelta * vec2(isTranslate)) + vec2(translateKeyDelta.w, translateKeyDelta.x) - vec2(translateKeyDelta.z, translateKeyDelta.y)) * modelTranslateSensitivity, 0);//记录总位移量
				}
				models[focusedModelIndex]->SetTransform
				(
					vec3(((mousePositionDelta * vec2(isTranslate)) + vec2(translateKeyDelta.w, translateKeyDelta.x) - vec2(translateKeyDelta.z, translateKeyDelta.y)) * modelTranslateSensitivity, 0),
					((mousePositionDelta * vec2(isRotate)) + vec2(rotateKeyDelta.w, rotateKeyDelta.x) - vec2(rotateKeyDelta.z, rotateKeyDelta.y)) * modelRotateSensitivity,
					vec3(1 + (mouseScrollDelta + scaleKeyDelta.y - scaleKeyDelta.x) * modelScaleSensitivity)
				);
				ResetMouseDelta();
				ResetKeyDelta();
			}
		}

		if (isUseNPR)
		{
			vec4 NPR_Color[4] =
			{
				vec4(NPRColor[0].x, NPRColor[0].y, NPRColor[0].z, NPRColor[0].w),
				vec4(NPRColor[1].x, NPRColor[1].y, NPRColor[1].z, NPRColor[1].w),
				vec4(NPRColor[2].x, NPRColor[2].y, NPRColor[2].z, NPRColor[2].w),
				vec4(NPRColor[3].x, NPRColor[3].y, NPRColor[3].z, NPRColor[3].w)
			};
			SetNPR(modelShader, NPRColorWeight, NPR_Color);
		}

		{
			ActiveandBindModelsTexture();
			DrawModels({ &model1,&model2,&model3,&model4,&model5,&model6 }, { 84,70,12,44,96,109 }, mainWindow, modelShader, outlineShader);
		}

		{
			SetTestEnable(GL_STENCIL_TEST, false);
			SetTestEnable(GL_DEPTH_TEST, true);

			SetImguiNewFrame();
			GenImguiMainWindow(modelShader, backGroundShader);
			RenderImgui();
		}

		glfwSwapBuffers(mainWindow.window);//交换缓冲
		glfwPollEvents();//检查是否有事件被触发并更新窗口状态以及调用对应的回调函数
	}

	/*………………………………程序结束后处理………………………………*/
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		glfwTerminate();
	}
	return 0;
}