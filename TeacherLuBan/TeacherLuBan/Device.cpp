#include "Device.h"



void RenderTeacherLuBan::Device::InitGLFW()
{
	glfwInit();//��ʼ��GLFW
	/*glfwWindowHint��������GLFW����һ��������ѡ�����ƣ��ڶ���������ѡ���һ�����ε�ֵ*/
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//�趨���汾��Ϊ3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//�趨�ΰ汾��Ϊ3
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//����GLFWʹ�ú���ģʽ
}

//void RenderTeacherLuBan::Device::InitGLAD()
//{
//	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))//glfwGetProcAddress�����ڼ���ϵͳ��ص�OpenGL����ָ���ַ�ĺ���(�������趨���߳�������֮�����)
//	{
//		cout << "��ʼ��GLADʧ��" << endl;
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
