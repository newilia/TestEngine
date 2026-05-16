#pragma once

#include <filesystem>
#include <optional>

#ifdef _WIN32
struct HWND__;
using HWND = HWND__*;
#endif

namespace Engine::EditorDialogs {

	struct SceneFileDialogOptions
	{
#ifdef _WIN32
		HWND parentHwnd = nullptr;
#endif
		std::filesystem::path initialDirectory;
		std::filesystem::path suggestedFileName;
	};

	[[nodiscard]] std::optional<std::filesystem::path> PickSceneXmlOpen(const SceneFileDialogOptions& opts);
	[[nodiscard]] std::optional<std::filesystem::path> PickSceneXmlSave(const SceneFileDialogOptions& opts);

} // namespace Engine::EditorDialogs
