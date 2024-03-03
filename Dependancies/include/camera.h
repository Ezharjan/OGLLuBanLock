#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
using namespace glm;

/*相机移动的枚举*/
enum Camera_Movement
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

/*相机参数默认值*/
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

class Camera
{
public:
	/*相机属性*/
	vec3 Position;
	vec3 Front;
	vec3 Up;
	vec3 Right;
	vec3 WorldUp;
	/*欧拉角*/
	float Yaw;
	float Pitch;
	/*其他选项*/
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	/*
	包含位置、上向量、偏航角、俯仰角的构造函数
	同时初始化前向量、相机移动速度、移动敏感度和缩放
	*/
	Camera(vec3 position = vec3(0.0f, 0.0f, 0.0f), vec3 up = vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}
	/*
	标量式构造函数
	*/
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = vec3(posX, posY, posZ);
		WorldUp = vec3(upX, upY, upZ);
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
	}

	/*获取视图矩阵*/
	mat4 GetViewMatrix()
	{
		return lookAt(Position, Position + Front, Up);
	}

	/*相机平移*/
	void ProcessKeyboard(Camera_Movement direction, float deltaTime)
	{
		float velocity = MovementSpeed * deltaTime;//相机移动速度通过deltaTime实现更顺滑的移动效果
		switch (direction)
		{
		case FORWARD:
			Position += Front * velocity;
			break;
		case BACKWARD:
			Position -= Front * velocity;
			break;
		case LEFT:
			Position -= Right * velocity;
			break;
		case RIGHT:
			Position += Right * velocity;
			break;
		default:
			break;
		}
	}

	/*相机旋转*/
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)//constrainPitch为是否限制俯仰角在+-90度之间
	{
		/*通过敏感度调整旋转速度*/
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		/*将鼠标移动和相机旋转对应起来*/
		Yaw += xoffset;
		Pitch += yoffset;

		/*限制俯仰角范围以防止视图翻转*/
		if (constrainPitch)
		{
			if (Pitch > 89.0f)
			{
				Pitch = 89.0f;
			}
			else if (Pitch < -89.0f)
			{
				Pitch = -89.0f;
			}
		}
		updateCameraVectors();
	}

	/*相机视角缩放*/
	void ProcessMouseScroll(float yoffset)
	{
		/*限制视角在0度值至45度之间*/
		if (Zoom >= 1.0f && Zoom <= 45.0f)
		{
			Zoom -= yoffset;
		}
		if (Zoom <= 1.0f)
		{
			Zoom = 1.0f;
		}
		if (Zoom >= 45.0f)
		{
			Zoom = 45.0f;
		}
	}

private:
	/*更新相机各方向的方向向量*/
	void updateCameraVectors()
	{
		/*计算相机前方向向量*/
		vec3 front;
		front.x = cos(radians(Yaw)) * cos(radians(Pitch));//x分量是向量在xz平面的投影的x轴方向上的投影
		front.y = sin(radians(Pitch));
		front.z = sin(radians(Yaw)) * cos(radians(Pitch));
		Front = normalize(front);//方向向量经标准化以忽略大小

		Right = normalize(cross(Front, WorldUp));//利用叉乘获得同时垂直前向量和世界坐标下上向量的相机右向量
		Up = normalize(cross(Right, Front));//利用叉乘获取同时垂直相机右向量和前向量的上向量
	}
};
#endif // !CAMERA_H