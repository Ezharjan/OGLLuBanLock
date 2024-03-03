#pragma once
#include <GLFW/glfw3.h>   // done



//class Device
//{
//public:
//	Device();
//	~Device();
//	float windowWidth;
//	float windowHeight;
//	GLFWwindow* window;
//	const char* windowTitle;
//
//private:
//	void SetCallback(GLFWwindow* window);
//};


namespace RenderTeacherLuBan {

	class Device
	{
	public:
		Device();
		~Device();
	protected:
		void InitGLFW();
		void InitGLAD();
		//void InitImgui(const Window& mainWindow);
	private:
	};
}