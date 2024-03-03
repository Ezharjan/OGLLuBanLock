#version 330 core
layout (location = 0) in vec3 vertPosition;//顶点坐标
layout (location = 1) in vec2 textureCoordinate;//纹理坐标

out vec2 fragTexture;//纹理坐标输出

uniform mat4 view;//视图矩阵
uniform mat4 projection;//透视矩阵

void main()
{
    fragTexture = textureCoordinate;    
    gl_Position = projection * view * vec4(vertPosition, 1.0f);
}