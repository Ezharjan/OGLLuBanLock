#version 330 core
in vec2 fragTexture;//Ƭ��������������

out vec4 FragColor;//Ƭ����ɫ���

uniform sampler2D backgroundTexture;//��������

void main()
{
    FragColor = texture(backgroundTexture,fragTexture);
}