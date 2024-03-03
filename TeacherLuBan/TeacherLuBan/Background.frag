#version 330 core
in vec2 fragTexture;//片段纹理坐标输入

out vec4 FragColor;//片段颜色输出

uniform sampler2D backgroundTexture;//背景纹理

void main()
{
    FragColor = texture(backgroundTexture,fragTexture);
}