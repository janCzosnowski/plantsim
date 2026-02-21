#pragma once
#include <string>

extern bool InitVideoBackground(const std::string& path);
extern void UpdateVideoBackground();
extern void DrawVideoBackground();
extern void ShutdownVideoBackground();