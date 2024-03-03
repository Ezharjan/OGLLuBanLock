#include "Device.h"



void RenderTeacherLuBan::Device::InitGLFW()
{
	glfwInit();//初始化GLFW
	/*glfwWindowHint用于配置GLFW，第一个参数是选项名称，第二个是设置选项的一个整形的值*/
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//设定主版本号为3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//设定次版本号为3
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//告诉GLFW使用核心模式
}

//void RenderTeacherLuBan::Device::InitGLAD()
//{
//	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))//glfwGetProcAddress是用于加载系统相关的OpenGL函数指针地址的函数(必须在设定好线程上下文之后调用)
//	{
//		cout << "初始化GLAD失败" << endl;
//		glfwTerminate();
//	}
//	gladLoadGL();
//}
//
//void RenderTeacherLuBan::Device::InitImgui(const Window& mainWindow)
//{
//	IMGUI_CHECKVERSION();
//	ImGui::CreateContext();
//	ImGuiIO& io = ImGui::GetIO(); (void)io;
//	ImGui::StyleColorsDark();
//	ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\Dengb.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
//	ImGui_ImplGlfw_InitForOpenGL(mainWindow.window, true);
//	ImGui_ImplOpenGL3_Init("#version 330");
//}
