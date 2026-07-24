#include "Engine/Editor/EditorPreferences.h"

#include <pugixml.hpp>

#include <algorithm>

namespace Engine {
	namespace {

		constexpr const char kRootElement[] = "EditorPreferences";
		constexpr const char kLoadLastSceneOnStartupElement[] = "LoadLastSceneOnStartup";
		constexpr const char kLastScenePathElement[] = "LastScenePath";
		constexpr const char kRecentDocumentsElement[] = "RecentDocuments";
		constexpr const char kRecentDocumentPathElement[] = "Path";

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

	void EditorPreferences::AddRecentDocument(const std::filesystem::path& path) {
		const std::optional<std::filesystem::path> normalizedPath = TryMakePathRelativeToContentRoot(path);
		if (!normalizedPath) {
			return;
		}

		const auto existing = std::find(_recentDocuments.begin(), _recentDocuments.end(), *normalizedPath);
		if (existing != _recentDocuments.end()) {
			_recentDocuments.erase(existing);
		}
		_recentDocuments.insert(_recentDocuments.begin(), *normalizedPath);
		if (_recentDocuments.size() > kMaxRecentDocuments) {
			_recentDocuments.resize(kMaxRecentDocuments);
		}
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

		if (const pugi::xml_node recentDocumentsNode = root.child(kRecentDocumentsElement)) {
			_recentDocuments.clear();
			for (const pugi::xml_node pathNode : recentDocumentsNode.children(kRecentDocumentPathElement)) {
				const std::string pathText = pathNode.text().as_string();
				if (pathText.empty()) {
					continue;
				}
				_recentDocuments.emplace_back(pathText);
				if (_recentDocuments.size() >= kMaxRecentDocuments) {
					break;
				}
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
		if (!_recentDocuments.empty()) {
			pugi::xml_node recentDocumentsNode = root.append_child(kRecentDocumentsElement);
			for (const std::filesystem::path& recentPath : _recentDocuments) {
				recentDocumentsNode.append_child(kRecentDocumentPathElement)
				    .text()
				    .set(recentPath.generic_string().c_str());
			}
		}

		const std::filesystem::path filePath = PreferencesFilePath();
		std::error_code ec;
		std::filesystem::create_directories(filePath.parent_path(), ec);
		(void)document.save_file(filePath.string().c_str());
	}

} // namespace Engine
