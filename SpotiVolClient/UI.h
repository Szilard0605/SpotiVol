#pragma once

#include "Window.h"

#include <functional>

class UI
{
public:
	static void BeginFrame(Window* window);
	static void RenderConnecting();
	static void RenderConnected();
	static void RenderClientList();
	static void RenderWindowOutline();
	static void EndFrame();

	static bool IsSliderHovered();

	static float GetVolumeLevel();
	static void SetVolumeLevel(const float level);

	static void SetOnVolumeChangeCallback(std::function<void(float)> callback);
	static void CallOnVolumeChangeCallback(float volumeLevel);
};

