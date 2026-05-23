#pragma once

#include <filesystem>
#include <string_view>

namespace Engine {

	inline constexpr std::string_view kDefaultSceneObjectRelative = "assets\\sceneObjects\\Untitled.xml";
	inline constexpr std::string_view kDefaultSceneRelative = "assets\\scenes\\Untitled.xml";

	/// Asset and content paths are relative to the process working directory (repo root when launched from IDE/CMake).
	[[nodiscard]] inline std::filesystem::path ContentRoot() {
		return std::filesystem::absolute(std::filesystem::current_path());
	}

	[[nodiscard]] inline std::filesystem::path ResolveContentPath(const std::filesystem::path& path) {
		if (path.is_absolute()) {
			return path;
		}
		return ContentRoot() / path;
	}

	[[nodiscard]] inline std::filesystem::path DefaultSceneObjectRelativePath() {
		return std::filesystem::path{kDefaultSceneObjectRelative};
	}

	[[nodiscard]] inline std::filesystem::path DefaultSceneObjectAbsolutePath() {
		return ResolveContentPath(DefaultSceneObjectRelativePath());
	}

	[[nodiscard]] inline std::filesystem::path DefaultSceneObjectsDirectory() {
		return DefaultSceneObjectAbsolutePath().parent_path();
	}

	[[nodiscard]] inline std::filesystem::path DefaultSceneRelativePath() {
		return std::filesystem::path{kDefaultSceneRelative};
	}

	[[nodiscard]] inline std::filesystem::path DefaultSceneAbsolutePath() {
		return ResolveContentPath(DefaultSceneRelativePath());
	}

	[[nodiscard]] inline std::filesystem::path DefaultScenesDirectory() {
		return DefaultSceneAbsolutePath().parent_path();
	}

} // namespace Engine
