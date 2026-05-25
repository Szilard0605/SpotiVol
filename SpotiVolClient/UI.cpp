#include "UI.h"

#include "imgui.h"

static Window* s_Window = nullptr;
static float s_VolumeLevel = 0.0f;
static std::function<void(float)> s_OnVolumeChangeCallback = nullptr;

void UI::RenderConnecting()
{
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoSavedSettings;

	ImGui::Begin("Root", nullptr, flags);
	ImGui::Text("Connecting to server...");
	ImGui::End();
}

void UI::RenderConnected()
{

	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoSavedSettings;
	
	//ImGui::ShowDemoWindow();

	ImGui::Begin("Root", nullptr, flags);
	ImGui::TextColored(ImVec4(0,1,0,1), "Connected");

	const ImVec2 sliderSize = { 30.0f, 120.0f };
	const ImVec2 regionAvail = ImGui::GetContentRegionAvail();
	ImVec2 pos = { regionAvail.x * 0.5f - sliderSize.x / 2, regionAvail.y * 0.5f - sliderSize.y * 0.5f};
	ImGui::SetCursorPos(pos);
	if(ImGui::VSliderFloat("##VolumeSlider", sliderSize, &s_VolumeLevel, 0.0f, 1.0f, ""))
	{
		if(s_OnVolumeChangeCallback)
			s_OnVolumeChangeCallback(s_VolumeLevel);
	}

	// Close button
	ImVec2 closeBttnSize = { 80.0f, 30.0f };
	float closeBttnPaddingY = 5.0f;

	float centerX = regionAvail.x * 0.5f - closeBttnSize.x * 0.5f;
	float centerY = regionAvail.y * 0.5f + sliderSize.y * 0.5f + closeBttnSize.y - closeBttnPaddingY;

	ImGui::SetCursorPos({ centerX, centerY });
	if(ImGui::Button("Close", closeBttnSize))
	{
		PostQuitMessage(0);
	}

	ImGui::End();

}

void UI::RenderWindowOutline()
{
	ImDrawList* draw = ImGui::GetForegroundDrawList();

	ImVec2 p = ImGui::GetMainViewport()->Pos;
	ImVec2 size = ImGui::GetMainViewport()->Size;

	float thickness = 2.0f;

	draw->AddRect(
		p,
		ImVec2(p.x + size.x, p.y + size.y),
		IM_COL32(255, 255, 255, 255),
		0.0f,
		0,
		thickness
	);
}

void UI::BeginFrame(Window* window)
{
	if (!window)
		return;

	s_Window = window;
	window->NewFrame();
}

void UI::EndFrame()
{
	s_Window->PresentFrame();
}

float UI::GetVolumeLevel()
{
	return s_VolumeLevel;
}

void UI::SetVolumeLevel(const float level)
{
	s_VolumeLevel = level;
}

void UI::SetOnVolumeChangeCallback(std::function<void(float)> callback)
{
	s_OnVolumeChangeCallback = callback;
}