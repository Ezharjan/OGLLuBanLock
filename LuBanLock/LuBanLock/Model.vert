#version 330 core

layout (location = 0) in vec3 vertPositionInput;//��������
layout (location = 1) in vec2 textureInput;//��������
layout (location = 2) in vec3 normalInput;//��������

out vec3 fragPosition;//Ƭ��λ�����
out vec2 fragTexture;//�����������
out vec3 fragNormal;//�������

uniform mat4 model;//ģ�;���
uniform mat4 view;//��ͼ����
uniform mat4 projection;//ͶӰ����

void main()
{	
	//�����һ���Ĳü��ռ�����
    gl_Position = (projection * view * model) * vec4(vertPositionInput, 1.0);

	fragPosition = vec3(model * vec4(vertPositionInput,1.0));//�������
	fragTexture = textureInput;//�����������
	fragNormal = mat3(transpose(inverse(model))) * normalInput;//����������߾���(ģ�;���ȡ����ת��)�����ķ��߷���
}