#include "Engine/Editor/NativeFileDialog.h"

#include "Engine/Core/ContentPaths.h"

#include <filesystem>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <objbase.h>
#include <shobjidl.h>
#endif

namespace Engine::EditorDialogs {

	namespace {

#ifdef _WIN32

		struct ComScope
		{
			bool needsUninit = false;

			ComScope() {
				const HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
				if (hr == S_OK) {
					needsUninit = true;
				}
			}

			~ComScope() {
				if (needsUninit) {
					CoUninitialize();
				}
			}

			ComScope(const ComScope&) = delete;
			ComScope& operator=(const ComScope&) = delete;
		};

		std::wstring ToWide(const std::filesystem::path& path) {
			return path.wstring();
		}

		bool SetDialogFolder(IFileDialog& dialog, const std::filesystem::path& directory) {
			if (directory.empty()) {
				return true;
			}
			std::error_code ec;
			std::filesystem::path folder = directory;
			if (folder.has_filename() && !std::filesystem::is_directory(folder, ec)) {
				folder = folder.parent_path();
			}
			if (folder.empty()) {
				return true;
			}
			IShellItem* item = nullptr;
			const HRESULT hr = SHCreateItemFromParsingName(ToWide(folder).c_str(), nullptr, IID_PPV_ARGS(&item));
			if (FAILED(hr) || !item) {
				return false;
			}
			const HRESULT setHr = dialog.SetFolder(item);
			item->Release();
			return SUCCEEDED(setHr);
		}

		bool SetDialogFileName(IFileDialog& dialog, const std::filesystem::path& fileName) {
			if (fileName.empty()) {
				return true;
			}
			const std::filesystem::path name = fileName.filename();
			if (name.empty()) {
				return true;
			}
			return SUCCEEDED(dialog.SetFileName(ToWide(name).c_str()));
		}

		std::optional<std::filesystem::path> PathFromDialogResult(IFileDialog& dialog) {
			IShellItem* item = nullptr;
			if (FAILED(dialog.GetResult(&item)) || !item) {
				return std::nullopt;
			}
			PWSTR widePath = nullptr;
			const HRESULT hr = item->GetDisplayName(SIGDN_FILESYSPATH, &widePath);
			item->Release();
			if (FAILED(hr) || !widePath) {
				return std::nullopt;
			}
			std::filesystem::path result{widePath};
			CoTaskMemFree(widePath);
			return result;
		}

		void ApplyCommonDialogOptions(IFileDialog& dialog, const SceneFileDialogOptions& opts) {
			constexpr COMDLG_FILTERSPEC filters[] = {
			    {L"Scene XML (*.xml)", L"*.xml"},
			    {L"All files (*.*)", L"*.*"},
			};
			(void)dialog.SetFileTypes(2, filters);
			(void)dialog.SetFileTypeIndex(1);
			(void)dialog.SetDefaultExtension(L"xml");
			(void)SetDialogFolder(dialog, opts.initialDirectory);
			(void)SetDialogFileName(dialog, opts.suggestedFileName);
		}

		std::optional<std::filesystem::path> PickSceneXml(IFileDialog& dialog, HWND parentHwnd) {
			const HRESULT hr = dialog.Show(parentHwnd);
			if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
				return std::nullopt;
			}
			if (FAILED(hr)) {
				return std::nullopt;
			}
			return PathFromDialogResult(dialog);
		}

#endif

	} // namespace

	std::optional<std::filesystem::path> PickSceneXmlOpen(const SceneFileDialogOptions& opts) {
#ifdef _WIN32
		ComScope com;
		IFileOpenDialog* dialog = nullptr;
		if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog))) ||
		    !dialog) {
			return std::nullopt;
		}
		dialog->SetOptions(FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST | FOS_FILEMUSTEXIST);
		SceneFileDialogOptions resolved = opts;
		if (resolved.initialDirectory.empty()) {
			resolved.initialDirectory = DefaultSceneObjectsDirectory();
		}
		if (resolved.suggestedFileName.empty()) {
			resolved.suggestedFileName = DefaultSceneObjectAbsolutePath();
		}
		ApplyCommonDialogOptions(*dialog, resolved);
		const auto result = PickSceneXml(*dialog, opts.parentHwnd);
		dialog->Release();
		return result;
#else
		(void)opts;
		return std::nullopt;
#endif
	}

	std::optional<std::filesystem::path> PickSceneXmlSave(const SceneFileDialogOptions& opts) {
#ifdef _WIN32
		ComScope com;
		IFileSaveDialog* dialog = nullptr;
		if (FAILED(CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog))) ||
		    !dialog) {
			return std::nullopt;
		}
		dialog->SetOptions(FOS_FORCEFILESYSTEM | FOS_OVERWRITEPROMPT);
		SceneFileDialogOptions resolved = opts;
		if (resolved.initialDirectory.empty()) {
			resolved.initialDirectory = DefaultSceneObjectsDirectory();
		}
		if (resolved.suggestedFileName.empty()) {
			resolved.suggestedFileName = DefaultSceneObjectRelativePath().filename();
		}
		ApplyCommonDialogOptions(*dialog, resolved);
		const auto result = PickSceneXml(*dialog, opts.parentHwnd);
		dialog->Release();
		return result;
#else
		(void)opts;
		return std::nullopt;
#endif
	}

} // namespace Engine::EditorDialogs
