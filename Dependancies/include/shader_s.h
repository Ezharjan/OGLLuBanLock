#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
using namespace std;
using namespace glm;
class Shader
{
public:
	unsigned int ID;//程序ID

	/*检测着色器编译是否成功*/
	void CheckShaderCompile(unsigned int vertexShader, unsigned int fragmentShader)
	{
		unsigned int shaders[] = { vertexShader ,fragmentShader };

		int success;//设定是否成功的标记
		for (int i = 0; i < 2; i++)
		{
			glGetShaderiv(shaders[i], GL_COMPILE_STATUS, &success);//检测编译状态
			if (!success)//如果编译不通过
			{
				char infoLog[512];//错误信息
				glGetShaderInfoLog(shaders[i], 512, NULL, infoLog);//记录错误信息
				switch (i)
				{
				case 0:
					cout << "顶点着色器发生编译错误：" << infoLog << endl;
					break;
				case 1:
					cout << "片段着色器发生编译错误：" << infoLog << endl;
					break;
				}
			}
		}
	}

	/*检测着色器链接是否成功*/
	void CheckShaderLink(unsigned int shaderProgram)
	{
		int success;//设定是否成功的标记
		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);//检测链接状态
		if (!success)
		{
			char infoLog[512];//错误信息
			glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);//记录错误信息
			cout << "着色器发生链接错误：" << infoLog << endl;
		}
	}

	Shader(const char* vertexPath, const char* fragmentPath)//构造器读取并构建着色器
	{
		/*1.从文件路径中获取顶点/片段着色器*/
		string vertexCode;
		string fragmentCode;
		ifstream vShaderFile;
		ifstream fShaderFile;
		// 保证ifstream对象可以抛出异常：
		vShaderFile.exceptions(ifstream::failbit | ifstream::badbit);
		fShaderFile.exceptions(ifstream::failbit | ifstream::badbit);
		try
		{
			// 打开文件
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);
			stringstream vShaderStream, fShaderStream;
			// 读取文件的缓冲内容到数据流中
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			// 关闭文件处理器
			vShaderFile.close();
			fShaderFile.close();
			// 转换数据流到string
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
		}
		catch (ifstream::failure e)
		{
			cout << "文件读取失败！" << endl;
		}
		const char* vShaderCode = vertexCode.c_str();
		const char* fShaderCode = fragmentCode.c_str();

		/*2.编译着色器*/
		unsigned int vertex, fragment;
		int success;
		char infoLog[512];

		/*顶点着色器*/
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);

		/*片段着色器*/
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);

		/*打印编译错误(如果有)*/
		CheckShaderCompile(vertex, fragment);

		/*着色器程序*/
		ID = glCreateProgram();
		glAttachShader(ID, vertex);
		glAttachShader(ID, fragment);
		glLinkProgram(ID);

		/*打印连接错误(如果有)*/
		CheckShaderLink(ID);

		/*删除已经链接到程序的，不需要的着色器*/
		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}

	void use()//激活着色器程序
	{
		glUseProgram(ID);
	}

	/*uniform工具函数*/
	void setBool(const string& name, bool value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
	}
	void setInt(const string& name, int value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
	}
	void setFloat(const string& name, float value) const
	{
		glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
	}
	void setVec2(const string& name, const vec2& value) const
	{
		glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec2(const string& name, float x, float y) const
	{
		glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
	}
	void setVec3(const string& name, const vec3& value) const
	{
		glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec3(const string& name, float x, float y, float z) const
	{
		glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
	}
	void setVec4(const string& name, const vec4& value) const
	{
		glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec4(const string& name, float x, float y, float z, float w)
	{
		glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
	}
	void setMat2(const string& name, const mat2& mat) const
	{
		glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	void setMat3(const string& name, const mat3& mat) const
	{
		glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	void setMat4(const string& name, const mat4& mat) const
	{
		glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
};

#endif // !SHADER_H