#pragma once
#include<windows.h>
#include "mmsystem.h"//��������ͷ�ļ�
#pragma comment(lib,"winmm.lib")//��������ͷ�ļ���
#include<stdio.h>


class SoundManager
{
public:
	LPCSTR filePath;
	SoundManager(LPCSTR pth) :filePath(pth) {};
	~SoundManager() {};
	void Play();
protected:
	void PlaySound(const LPCSTR filePath);
private:
};


