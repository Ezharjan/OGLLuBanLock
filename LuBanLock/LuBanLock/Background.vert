#version 330 core
layout (location = 0) in vec3 vertPosition;//��������
layout (location = 1) in vec2 textureCoordinate;//��������

out vec2 fragTexture;//�����������

uniform mat4 view;//��ͼ����
uniform mat4 projection;//͸�Ӿ���

void main()
{
    fragTexture = textureCoordinate;    
    gl_Position = projection * view * vec4(vertPosition, 1.0f);
}