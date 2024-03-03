#pragma once

#include "InteractionManager.h"
#include "Models.h"


void Render() {
	InitGLFW();
	Window mainWindow;
	InitGLAD();
	InitImgui(mainWindow);

	Shader modelShader("Model.vert", "Model.frag");//ģ����ɫ��
	Shader backGroundShader("Background.vert", "Background.frag");//������ɫ��
	Shader outlineShader("Outline.vert", "Outline.frag");//�����ɫ��

	SetViewMatrix();

	//�������
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	//��ʼ��ģ��

	ModelStruct modelStruct = GetModel();

	//��ʼ��ģ�ͽ���

	InitModelTexture(modelShader, "Resources/Textures/Diffuse/05.png", "Resources/Textures/Specular/05.png");
	InitModelTexture(backGroundShader, backgroundPath);

	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);//��ģ�����Ȳ��Բ�ͨ��������ǰģ��ֵ������ģ��ֵ���趨Ϊ1

	InitModels({ &modelStruct.model1,&modelStruct.model2, &modelStruct.model3, &modelStruct.model4, &modelStruct.model5, &modelStruct.model6 });

	InitAnimationSequence({ &modelStruct.model1,&modelStruct.model2, &modelStruct.model3, &modelStruct.model4, &modelStruct.model5, &modelStruct.model6 }, { 2,4,3,5,0,1 });

	InitRecordedModels();

	/*��������������������������Ⱦѭ��������������������������*/
	while (!glfwWindowShouldClose(mainWindow.window))//������û�йر�
	{
		ClearAllBuffer();

		GetDeltaTime();

		SetProjectionMatrix(mainWindow, isOrtho, fov);

		{
			ActiveandBindBackgroundTexture();
			RenderBackground(backGroundShader, modelStruct.background);
		}

		if (!isLockLightPoiiton)//����������ƹ�λ��
		{
			lightPosition = vec3((lastmousePosition.x - (1920.0f / 2)) / (1920.0f / 2), (lastmousePosition.y - (1080.0f / 2)) / (-1080.0f / 2), lightDepth);//ʹ�ƹ����������ƶ�
		}
		GenSpotLight(modelShader, lightPosition, vec3(0, 0, -1), innerCutoffAngle, outerCutoffAngle);
		SetLightProperties(modelShader, vec4(ambient.x, ambient.y, ambient.z, ambient.w), vec4(diffuse.x, diffuse.y, diffuse.z, diffuse.w), vec4(specular.x, specular.y, specular.z, specular.w), pow(2, shininess), 1.0f, 0.0014f, 0.000007f);

		//�ж��Ƿ���ģ�ͱ��۽�
		{
			isAnyModelFocused = false;//�ٶ�û��ģ�ͱ��۽�
			if (focusedModelIndex != -1)//������ھ۽�ģ������
			{
				isAnyModelFocused = true;
			}
		}

		if (!isAnyModelFocused)//���û�����屻�۽�
		{
			if (isRecordedModelMartix)
			{
				for (size_t i = 0; i < models.size(); i++)
				{
					models[i]->model = modelsforRecord[i];//��ԭ����ģ�͵�ģ�;���
				}
				translateSum = translateSumforRecord;
				isRecordedModelMartix = false;
			}

			PlayAnimation(animationTime, modelShader);

			if (!ImGui::IsAnyItemActive() && !ImGui::IsAnyWindowHovered())//��ȥ�����ImGui�ϵ����
			{
				if (isTranslate)//�������λ��
				{
					translateSum += vec3(((mousePositionDelta * vec2(isTranslate)) + vec2(translateKeyDelta.w, translateKeyDelta.x) - vec2(translateKeyDelta.z, translateKeyDelta.y)) * modelTranslateSensitivity, 0);//��¼��λ����
				}
				SetAllModelsTransform
				(
					vec3(((mousePositionDelta * vec2(isTranslate)) + vec2(translateKeyDelta.w, translateKeyDelta.x) - vec2(translateKeyDelta.z, translateKeyDelta.y)) * modelTranslateSensitivity, 0),
					((mousePositionDelta * vec2(isRotate)) + vec2(rotateKeyDelta.w, rotateKeyDelta.x) - vec2(rotateKeyDelta.z, rotateKeyDelta.y)) * modelRotateSensitivity,
					vec3(1 + (mouseScrollDelta + scaleKeyDelta.y - scaleKeyDelta.x) * modelScaleSensitivity)
				);
				ResetMouseDelta();
				ResetKeyDelta();
			}
		}
		else//������������屻�۽�
		{
			if (!isRecordedModelMartix)
			{
				for (size_t i = 0; i < models.size(); i++)
				{
					modelsforRecord[i] = models[i]->model;//��¼����ģ�͵�ģ�;���
				}
				translateSumforRecord = translateSum;//��¼����λ������
				models[focusedModelIndex]->ResetTransform();
				isRecordedModelMartix = true;
			}
			if (!ImGui::IsAnyItemActive() && !ImGui::IsAnyWindowHovered())//��ȥ�����ImGui�ϵ����
			{
				if (isTranslate)//�������λ��
				{
					translateSum += vec3(((mousePositionDelta * vec2(isTranslate)) + vec2(translateKeyDelta.w, translateKeyDelta.x) - vec2(translateKeyDelta.z, translateKeyDelta.y)) * modelTranslateSensitivity, 0);//��¼��λ����
				}
				models[focusedModelIndex]->SetTransform
				(
					vec3(((mousePositionDelta * vec2(isTranslate)) + vec2(translateKeyDelta.w, translateKeyDelta.x) - vec2(translateKeyDelta.z, translateKeyDelta.y)) * modelTranslateSensitivity, 0),
					((mousePositionDelta * vec2(isRotate)) + vec2(rotateKeyDelta.w, rotateKeyDelta.x) - vec2(rotateKeyDelta.z, rotateKeyDelta.y)) * modelRotateSensitivity,
					vec3(1 + (mouseScrollDelta + scaleKeyDelta.y - scaleKeyDelta.x) * modelScaleSensitivity)
				);
				ResetMouseDelta();
				ResetKeyDelta();
			}
		}

		if (isUseNPR)
		{
			vec4 NPR_Color[4] =
			{
				vec4(NPRColor[0].x, NPRColor[0].y, NPRColor[0].z, NPRColor[0].w),
				vec4(NPRColor[1].x, NPRColor[1].y, NPRColor[1].z, NPRColor[1].w),
				vec4(NPRColor[2].x, NPRColor[2].y, NPRColor[2].z, NPRColor[2].w),
				vec4(NPRColor[3].x, NPRColor[3].y, NPRColor[3].z, NPRColor[3].w)
			};
			SetNPR(modelShader, NPRColorWeight, NPR_Color);
		}

		{
			ActiveandBindModelsTexture();
			DrawModels({ &modelStruct.model1,&modelStruct.model2,&modelStruct.model3,&modelStruct.model4,&modelStruct.model5,&modelStruct.model6 }, { 84,70,12,44,96,109 }, mainWindow, modelShader, outlineShader);
		}

		{
			SetTestEnable(GL_STENCIL_TEST, false);
			SetTestEnable(GL_DEPTH_TEST, true);

			SetImguiNewFrame();
			GenImguiMainWindow(modelShader, backGroundShader);
			RenderImgui();
		}

		glfwSwapBuffers(mainWindow.window);//��������
		glfwPollEvents();//����Ƿ����¼������������´���״̬�Լ����ö�Ӧ�Ļص�����
	}

	/*���������������������������������������������������������*/
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		glfwTerminate();
	}
}