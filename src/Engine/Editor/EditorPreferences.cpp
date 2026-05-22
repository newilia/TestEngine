#include "Engine/Editor/EditorPreferences.h"

#include <pugixml.hpp>

namespace Engine {
	namespace {

		constexpr const char kRootElement[] = "EditorPreferences";
		constexpr const char kLoadLastSceneOnStartupElement[] = "LoadLastSceneOnStartup";
		constexpr const char kLastScenePathElement[] = "LastScenePath";

		[[nodiscard]] std::optional<std::filesystem::path> TryMakePathRelativeToContentRoot(
		    const std::filesystem::path& path) {
			if (path.empty()) {
				return std::nullopt;
			}
			const std::filesystem::path absolute = path.is_absolute() ? path : ResolveContentPath(path);
			std::error_code ec;
			const std::filesystem::path relative = std::filesystem::relative(absolute, ContentRoot(), ec);
			const std::string relativeText = relative.generic_string();
			if (!ec && !relative.empty() && relativeText.find("..") == std::string::npos) {
				return relative;
			}
			return absolute;
		}

	} // namespace

	std::filesystem::path EditorPreferences::PreferencesFilePath() {
		return ResolveContentPath("editor_preferences.xml");
	}

	EditorPreferences::EditorPreferences() {
		LoadFromDisk();
	}

	void EditorPreferences::SetLastScenePath(std::optional<std::filesystem::path> path) {
		if (!path) {
			_lastScenePath.reset();
			return;
		}
		_lastScenePath = TryMakePathRelativeToContentRoot(*path);
	}

	void EditorPreferences::LoadFromDisk() {
		const std::filesystem::path filePath = PreferencesFilePath();
		if (!std::filesystem::exists(filePath)) {
			return;
		}

		pugi::xml_document document;
		if (!document.load_file(filePath.string().c_str())) {
			return;
		}

		const pugi::xml_node root = document.child(kRootElement);
		if (!root) {
			return;
		}

		if (const pugi::xml_node loadLastNode = root.child(kLoadLastSceneOnStartupElement)) {
			_isLoadLastSceneOnStartup = loadLastNode.text().as_bool();
		}

		if (const pugi::xml_node lastPathNode = root.child(kLastScenePathElement)) {
			const std::string pathText = lastPathNode.text().as_string();
			if (!pathText.empty()) {
				_lastScenePath = std::filesystem::path{pathText};
			}
		}
	}

	void EditorPreferences::Save() const {
		pugi::xml_document document;
		pugi::xml_node root = document.append_child(kRootElement);
		root.append_child(kLoadLastSceneOnStartupElement).text().set(_isLoadLastSceneOnStartup);
		if (_lastScenePath) {
			root.append_child(kLastScenePathElement).text().set(_lastScenePath->generic_string().c_str());
		}

		const std::filesystem::path filePath = PreferencesFilePath();
		std::error_code ec;
		std::filesystem::create_directories(filePath.parent_path(), ec);
		(void)document.save_file(filePath.string().c_str());
	}

} // namespace Engine
