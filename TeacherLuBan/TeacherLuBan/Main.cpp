#include "RenderLoop.h"

int main()
{
	SoundManager mp3 = SoundManager("E:\\1.mp3");
	mp3.Play();
	Render();
	return 0;
}