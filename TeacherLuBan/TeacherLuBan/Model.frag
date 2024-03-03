#version 330 core

in vec3 fragPosition;//片段位置输出
in vec2 fragTexture;//纹理坐标输出
in vec3 fragNormal;//法线输出

out vec4 FragColor;//片元颜色输出

/*材质结构体*/
struct Material
{
	sampler2D diffuse;//漫反射贴图采样器
	sampler2D specular;//高光贴图采样器
	float shininess;//光泽度
};
uniform Material material;

/*灯光结构体*/
struct Light
{
	vec3 position;//位置
    vec3 direction;//方向
 
    vec4 ambient;//环境光
    vec4 diffuse;//漫反射光
    vec4 specular;//高光
	
	float innerCutOff;//聚光灯内光切
    float outerCutOff;//聚光灯外光切

    float constant;//衰减常量
    float linear;//衰减线性(一次项)
    float quadratic;//衰减二次项
};
uniform Light light;

uniform vec3 viewPosition;//视图位置

/*NPR结构体*/
struct NPR
{
	bool isUseNPR;//是否使用卡通渲染
	//NPR使用的颜色比重
	float NPRColorWeight0;
	float NPRColorWeight1;
	float NPRColorWeight2;
	float NPRColorWeight3;
	//NPR使用的颜色
	vec4 NPRColor0;
	vec4 NPRColor1;
	vec4 NPRColor2;
	vec4 NPRColor3;
};
uniform NPR npr;//非真实感渲染

void main()
{
	//法线方向向量
	vec3 normalDirection = normalize(fragNormal);

	//片段指向光源的方向向量
	vec3 lightDirection = normalize(light.position - fragPosition);

	//设置环境光
	vec4 ambient = light.ambient * texture(material.diffuse,fragTexture);//纹理基色和环境光色的混合

	//设置漫反射
	vec4 diffuse = max(dot(normalDirection,lightDirection),0) * light.diffuse * texture(material.diffuse,fragTexture);//漫反射系数(灯的方向和法线方向点乘结果，角度越大值越小(非负))、漫反射光和纹理基色的混合

	//设置高光
	vec3 viewDirection = normalize(viewPosition - fragPosition);//片段指向视图方向
    vec3 reflectDirection = reflect(-lightDirection, normalDirection);//获得由光源指向片段向量的关于片段法向量反射后的方向向量	
    float specularWeight = pow(max(dot(viewDirection, reflectDirection), 0.0), material.shininess);//高光分量为视线方向和反射方向的点乘结果(非负)的反光度次方
    vec4 specular = specularWeight * light.specular * texture(material.specular, fragTexture);//高光分量、高光以及纹理基色的混合

	//点光源
    float angle = dot(lightDirection, normalize(-light.direction));//光锥角度为灯的照射方向和灯指向片段方向的夹角
    float epsilon = (light.innerCutOff - light.outerCutOff);//内和外光切之间的差
    float intensity = clamp((angle - light.outerCutOff) / epsilon, 0.0, 1.0);//基于角度和余弦差计算光强，钳制强度在0-1之间
    diffuse  *= intensity;//影响漫反射
    specular *= intensity;//影响高光

	//光衰减
    float distance = length(light.position - fragPosition);//获取片段和灯之间的距离	
    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));//计算衰减
    ambient  *= attenuation;//影响环境光
    diffuse  *= attenuation;//影响漫反射
    specular *= attenuation;//影响高光

	vec4 solidColor;//纯色
	if(npr.isUseNPR)
	{
		float intensityAngle = max(dot(lightDirection,normalDirection), 0);//得到灯光方向与法线方向之间的点乘结果，角度越大值值越小
		//根据不同比重，设定不同的纯色
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

	//输出颜色
	FragColor = (npr.isUseNPR) ? solidColor : (ambient + diffuse + specular);
}