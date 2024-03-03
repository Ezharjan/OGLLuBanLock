#pragma once

#include "InteractionManager.h"
#include "Models.h"


void Render() {
	InitGLFW();
	Window mainWindow;
	InitGLAD();
	InitImgui(mainWindow);

	Shader modelShader("Model.vert", "Model.frag");//模型着色器
	Shader backGroundShader("Background.vert", "Background.frag");//背景着色器
	Shader outlineShader("Outline.vert", "Outline.frag");//描边着色器

	SetViewMatrix();

	//开启混合
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	//初始化模型

	ModelStruct modelStruct = GetModel();

	//初始化模型结束

	InitModelTexture(modelShader, "Resources/Textures/Diffuse/05.png", "Resources/Textures/Specular/05.png");
	InitModelTexture(backGroundShader, backgroundPath);

	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);//当模板或深度测试不通过则保留当前模板值，否则模板值会设定为1

	InitModels({ &modelStruct.model1,&modelStruct.model2, &modelStruct.model3, &modelStruct.model4, &modelStruct.model5, &modelStruct.model6 });

	InitAnimationSequence({ &modelStruct.model1,&modelStruct.model2, &modelStruct.model3, &modelStruct.model4, &modelStruct.model5, &modelStruct.model6 }, { 2,4,3,5,0,1 });

	InitRecordedModels();

	/*………………………………渲染循环………………………………*/
	while (!glfwWindowShouldClose(mainWindow.window))//当窗口没有关闭
	{
		ClearAllBuffer();

		GetDeltaTime();

		SetProjectionMatrix(mainWindow, isOrtho, fov);

		{
			ActiveandBindBackgroundTexture();
			RenderBackground(backGroundShader, modelStruct.background);
		}

		if (!isLockLightPoiiton)//如果不锁定灯光位置
		{
			lightPosition = vec3((lastmousePosition.x - (1920.0f / 2)) / (1920.0f / 2), (lastmousePosition.y - (1080.0f / 2)) / (-1080.0f / 2), lightDepth);//使灯光随鼠标进行移动
		}
		GenSpotLight(modelShader, lightPosition, vec3(0, 0, -1), innerCutoffAngle, outerCutoffAngle);
		SetLightProperties(modelShader, vec4(ambient.x, ambient.y, ambient.z, ambient.w), vec4(diffuse.x, diffuse.y, diffuse.z, diffuse.w), vec4(specular.x, specular.y, specular.z, specular.w), pow(2, shininess), 1.0f, 0.0014f, 0.000007f);

		//判断是否有模型被聚焦
		{
			isAnyModelFocused = false;//假定没有模型被聚焦
			if (focusedModelIndex != -1)//如果存在聚焦模型索引
			{
				isAnyModelFocused = true;
			}
		}

		if (!isAnyModelFocused)//如果没有物体被聚焦
		{
			if (isRecordedModelMartix)
			{
				for (size_t i = 0; i < models.size(); i++)
				{
					models[i]->model = modelsforRecord[i];//还原所有模型的模型矩阵
				}
				translateSum = translateSumforRecord;
				isRecordedModelMartix = false;
			}

			PlayAnimation(animationTime, modelShader);

			if (!ImGui::IsAnyItemActive() && !ImGui::IsAnyWindowHovered())//除去鼠标在ImGui上的情况
			{
				if (isTranslate)//如果进行位移
				{
					translateSum += vec3(((mousePositionDelta * vec2(isTranslate)) + vec2(translateKeyDelta.w, translateKeyDelta.x) - vec2(translateKeyDelta.z, translateKeyDelta.y)) * modelTranslateSensitivity, 0);//记录总位移量
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
		else//如果有任意物体被聚焦
		{
			if (!isRecordedModelMartix)
			{
				for (size_t i = 0; i < models.size(); i++)
				{
					modelsforRecord[i] = models[i]->model;//记录所有模型的模型矩阵
				}
				translateSumforRecord = translateSum;//记录物体位移总量
				models[focusedModelIndex]->ResetTransform();
				isRecordedModelMartix = true;
			}
			if (!ImGui::IsAnyItemActive() && !ImGui::IsAnyWindowHovered())//除去鼠标在ImGui上的情况
			{
				if (isTranslate)//如果进行位移
				{
					translateSum += vec3(((mousePositionDelta * vec2(isTranslate)) + vec2(translateKeyDelta.w, translateKeyDelta.x) - vec2(translateKeyDelta.z, translateKeyDelta.y)) * modelTranslateSensitivity, 0);//记录总位移量
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

		glfwSwapBuffers(mainWindow.window);//交换缓冲
		glfwPollEvents();//检查是否有事件被触发并更新窗口状态以及调用对应的回调函数
	}

	/*………………………………程序结束后处理………………………………*/
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		glfwTerminate();
	}
}