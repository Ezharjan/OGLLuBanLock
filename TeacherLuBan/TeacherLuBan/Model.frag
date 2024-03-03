#version 330 core

in vec3 fragPosition;//Ƭ��λ�����
in vec2 fragTexture;//�����������
in vec3 fragNormal;//�������

out vec4 FragColor;//ƬԪ��ɫ���

/*���ʽṹ��*/
struct Material
{
	sampler2D diffuse;//��������ͼ������
	sampler2D specular;//�߹���ͼ������
	float shininess;//�����
};
uniform Material material;

/*�ƹ�ṹ��*/
struct Light
{
	vec3 position;//λ��
    vec3 direction;//����
 
    vec4 ambient;//������
    vec4 diffuse;//�������
    vec4 specular;//�߹�
	
	float innerCutOff;//�۹���ڹ���
    float outerCutOff;//�۹�������

    float constant;//˥������
    float linear;//˥������(һ����)
    float quadratic;//˥��������
};
uniform Light light;

uniform vec3 viewPosition;//��ͼλ��

/*NPR�ṹ��*/
struct NPR
{
	bool isUseNPR;//�Ƿ�ʹ�ÿ�ͨ��Ⱦ
	//NPRʹ�õ���ɫ����
	float NPRColorWeight0;
	float NPRColorWeight1;
	float NPRColorWeight2;
	float NPRColorWeight3;
	//NPRʹ�õ���ɫ
	vec4 NPRColor0;
	vec4 NPRColor1;
	vec4 NPRColor2;
	vec4 NPRColor3;
};
uniform NPR npr;//����ʵ����Ⱦ

void main()
{
	//���߷�������
	vec3 normalDirection = normalize(fragNormal);

	//Ƭ��ָ���Դ�ķ�������
	vec3 lightDirection = normalize(light.position - fragPosition);

	//���û�����
	vec4 ambient = light.ambient * texture(material.diffuse,fragTexture);//�����ɫ�ͻ�����ɫ�Ļ��

	//����������
	vec4 diffuse = max(dot(normalDirection,lightDirection),0) * light.diffuse * texture(material.diffuse,fragTexture);//������ϵ��(�Ƶķ���ͷ��߷����˽�����Ƕ�Խ��ֵԽС(�Ǹ�))���������������ɫ�Ļ��

	//���ø߹�
	vec3 viewDirection = normalize(viewPosition - fragPosition);//Ƭ��ָ����ͼ����
    vec3 reflectDirection = reflect(-lightDirection, normalDirection);//����ɹ�Դָ��Ƭ�������Ĺ���Ƭ�η����������ķ�������	
    float specularWeight = pow(max(dot(viewDirection, reflectDirection), 0.0), material.shininess);//�߹����Ϊ���߷���ͷ��䷽��ĵ�˽��(�Ǹ�)�ķ���ȴη�
    vec4 specular = specularWeight * light.specular * texture(material.specular, fragTexture);//�߹�������߹��Լ������ɫ�Ļ��

	//���Դ
    float angle = dot(lightDirection, normalize(-light.direction));//��׶�Ƕ�Ϊ�Ƶ����䷽��͵�ָ��Ƭ�η���ļн�
    float epsilon = (light.innerCutOff - light.outerCutOff);//�ں������֮��Ĳ�
    float intensity = clamp((angle - light.outerCutOff) / epsilon, 0.0, 1.0);//���ڽǶȺ����Ҳ�����ǿ��ǯ��ǿ����0-1֮��
    diffuse  *= intensity;//Ӱ��������
    specular *= intensity;//Ӱ��߹�

	//��˥��
    float distance = length(light.position - fragPosition);//��ȡƬ�κ͵�֮��ľ���	
    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));//����˥��
    ambient  *= attenuation;//Ӱ�컷����
    diffuse  *= attenuation;//Ӱ��������
    specular *= attenuation;//Ӱ��߹�

	vec4 solidColor;//��ɫ
	if(npr.isUseNPR)
	{
		float intensityAngle = max(dot(lightDirection,normalDirection), 0);//�õ��ƹⷽ���뷨�߷���֮��ĵ�˽�����Ƕ�Խ��ֵֵԽС
		//���ݲ�ͬ���أ��趨��ͬ�Ĵ�ɫ
		if(intensityAngle > npr.NPRColorWeight0)
		{
			solidColor = npr.NPRColor0;
		}
		else if(intensityAngle > npr.NPRColorWeight1)
		{
			solidColor = npr.NPRColor1;
		}
		else if(intensityAngle > npr.NPRColorWeight2)
		{
			solidColor = npr.NPRColor2;
		}
		else if(intensityAngle > npr.NPRColorWeight3)
		{
			solidColor = npr.NPRColor3;
		}
	}

	//�����ɫ
	FragColor = (npr.isUseNPR) ? solidColor : (ambient + diffuse + specular);
}