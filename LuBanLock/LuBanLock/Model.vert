#version 330 core

layout (location = 0) in vec3 vertPositionInput;//顶点输入
layout (location = 1) in vec2 textureInput;//纹理输入
layout (location = 2) in vec3 normalInput;//法线输入

out vec3 fragPosition;//片段位置输出
out vec2 fragTexture;//纹理坐标输出
out vec3 fragNormal;//法线输出

uniform mat4 model;//模型矩阵
uniform mat4 view;//视图矩阵
uniform mat4 projection;//投影矩阵

void main()
{	
	//定义归一化的裁剪空间坐标
    gl_Position = (projection * view * model) * vec4(vertPositionInput, 1.0);

	fragPosition = vec3(model * vec4(vertPositionInput,1.0));//输出顶点
	fragTexture = textureInput;//输出纹理坐标
	fragNormal = mat3(transpose(inverse(model))) * normalInput;//输出经过法线矩阵(模型矩阵取逆再转置)处理后的法线方向
}