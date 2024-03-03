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


//光标位置回调函数
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
	mousePositionDelta = vec2(xpos - lastmousePosition.x, lastmousePosition.y - ypos);//取得鼠标位置差值
	lastmousePosition = vec2(xpos, ypos);//记录该帧的鼠标位置
}

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

//鼠标滚轮回调函数
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	mouseScrollDelta = yoffset;
	isScale = (yoffset != 0);
}


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






///////////////////////////////////////////////////////////////////////////
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



/*………………模型结构体………………*/
struct MyModel
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
			surroundBoxs.push_back(SurroundBox(surroundPoints[i]));//根据包围点生成包围盒
		}

		this->animationPoint.push_back(vec3(0));
		this->animationPoint.insert(this->animationPoint.end(), animationPoint.begin(), animationPoint.end());//初始化动画点
	}

	//初始化顶点数据
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
		case MyModel::ObjectType::VAO:
			glBindVertexArray(VAO);//绑定VAO
			break;
		case MyModel::ObjectType::VBO:
			glBindBuffer(GL_ARRAY_BUFFER, VBO);//绑定VBO
			break;
		case MyModel::ObjectType::EBO:
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

vector<MyModel*> modelsbyAnimationSequence;//按照动画顺序排列的模型数组
//初始化动画顺序
void InitAnimationSequence(vector<MyModel*> _models, vector<int> animation_sequence)
{
	for (size_t i = 0; i < _models.size(); i++)
	{
		modelsbyAnimationSequence.push_back(_models[animation_sequence[i]]);//根据顺序记录所有模型
	}
}
vector<MyModel*>models;//模型数组
//初始化模型数组
void InitModels(vector<MyModel*>_models)
{
	for (MyModel* model : _models)
	{
		models.push_back(model);
	}
}
vec3 translateSumforRecord;//用于记录的平移之和
vector<mat4> modelsforRecord;//用于记录的模型矩阵数组
//初始化记录矩阵
void InitRecordedModels()
{
	for (MyModel* model : models)
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
void ConfigModel(MyModel model)
{
	model.ConfigAttribute(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	model.ConfigAttribute(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	model.ConfigAttribute(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	model.UnBindObject();
}
//设置所有模型变换
void SetAllModelsTransform(vec3 translateDelta, vec2 RotateAngle, vec3 scaleDelta)
{
	for (MyModel* model : models)
	{
		model->SetTransform(translateDelta, RotateAngle, scaleDelta);
	}
}

//绘制描边
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

int outlineIndex = -1;//需要描边物体的索引
bool isAnyModelFocused = false;//是否有物体被聚焦
int focusedModelIndex = -1;//被聚焦的物体索引
bool isRecordedModelMartix = false;//是否记录了模型矩阵
//绘制模型
void DrawModels(vector<MyModel*> models, vector<int> trianglesNumber, Window mainWindow, Shader modelShader, Shader outlineShader)
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

