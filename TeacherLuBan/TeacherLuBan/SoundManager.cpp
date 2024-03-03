#include "SoundManager.h"
#include <assert.h>

void SoundManager::Play()
{
	assert(this->filePath);
	this->PlaySound(this->filePath);
}

void SoundManager::PlaySound(const LPCSTR filePath)
{
	char buf[128];
	char str[128] = { 0 };
	int i = 0;

	//use mciSendCommand
	MCI_OPEN_PARMS mciOpen;
	MCIERROR mciError;
	//SetWindowText(NULL,"12345");
	mciOpen.lpstrDeviceType = "mpegvideo";
	mciOpen.lpstrElementName = filePath; // wav file is also supported
	mciError = mciSendCommand(0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD)&mciOpen);
	if (mciError)
	{
		mciGetErrorString(mciError, buf, 128);
		printf("send MCI_OPEN command failed:%s\n", buf);
		return;
	}
	UINT DeviceID = mciOpen.wDeviceID;
	MCI_PLAY_PARMS mciPlay;

	mciError = mciSendCommand(DeviceID, MCI_PLAY, 0, (DWORD)&mciPlay);
	if (mciError)
	{
		printf("send MCI_PLAY command failed\n");
		return;
	}
}