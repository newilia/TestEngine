#pragma once

#include "Engine/Core/ContentPaths.h"
#include "Engine/Core/Singleton.h"

#include <filesystem>
#include <optional>

namespace Engine {

	class EditorPreferences : public Singleton<EditorPreferences>
	{
		friend class Singleton<EditorPreferences>;

	public:
		[[nodiscard]] bool IsLoadLastSceneOnStartup() const {
			return _isLoadLastSceneOnStartup;
		}

		[[nodiscard]] bool& LoadLastSceneOnStartupMutable() {
			return _isLoadLastSceneOnStartup;
		}

		[[nodiscard]] std::optional<std::filesystem::path> GetLastScenePath() const {
			return _lastScenePath;
		}

		void SetLastScenePath(std::optional<std::filesystem::path> path);
		void Save() const;

	private:
		EditorPreferences();

		void LoadFromDisk();

		[[nodiscard]] static std::filesystem::path PreferencesFilePath();

		bool _isLoadLastSceneOnStartup = false;
		std::optional<std::filesystem::path> _lastScenePath;
	};

} // namespace Engine
