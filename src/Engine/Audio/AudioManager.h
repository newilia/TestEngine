#pragma once

#include <filesystem>
#include <memory>
#include <span>
#include <string_view>

namespace Engine {

	struct EventParameter
	{
		std::string_view name;
		float value = 0.f;
	};

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
		bool PlayEvent(std::string_view eventPath, std::span<const EventParameter> parameters);
		void StopAllEvents();

		bool IsInitialized() const;

	private:
		struct Impl;
		std::unique_ptr<Impl> _impl;
	};

} // namespace Engine
