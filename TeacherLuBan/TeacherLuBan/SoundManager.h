#pragma once
#include<windows.h>
#include "mmsystem.h"//导入声音头文件
#pragma comment(lib,"winmm.lib")//导入声音头文件库
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


