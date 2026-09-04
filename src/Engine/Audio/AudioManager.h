#pragma once

#include <filesystem>
#include <memory>
#include <string_view>

namespace Engine {

	class AudioManager
	{
	public:
		AudioManager();
		~AudioManager();

		AudioManager(const AudioManager&) = delete;
		AudioManager& operator=(const AudioManager&) = delete;
		AudioManager(AudioManager&&) = delete;
		AudioManager& operator=(AudioManager&&) = delete;

		bool Init();
		void Shutdown();
		void Update();

		bool LoadBank(const std::filesystem::path& bankPath);
		void UnloadAllBanks();
		bool PlayEvent(std::string_view eventPath);
		void StopAllEvents();

		bool IsInitialized() const;

	private:
		struct Impl;
		std::unique_ptr<Impl> _impl;
	};

} // namespace Engine
