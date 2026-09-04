#include "Engine/Audio/AudioManager.h"

#include "Engine/Core/ContentPaths.h"

#include <fmod_errors.h>
#include <fmod_studio.hpp>
#include <fmt/format.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <objbase.h>
#endif

#include <string>
#include <vector>

namespace Engine {

	namespace {
		void LogFmodError(std::string_view what, FMOD_RESULT result) {
			fmt::print(stderr, "[Audio] {}: {} ({})\n", what, FMOD_ErrorString(result), static_cast<int>(result));
		}

		bool EndsWithStringsBank(const std::filesystem::path& path) {
			const std::string name = path.filename().string();
			constexpr std::string_view suffix = ".strings.bank";
			if (name.size() < suffix.size()) {
				return false;
			}
			return name.compare(name.size() - suffix.size(), suffix.size(), suffix.data()) == 0;
		}
	} // namespace

	struct AudioManager::Impl
	{
		FMOD::Studio::System* system = nullptr;
		std::vector<FMOD::Studio::Bank*> banks;
#ifdef _WIN32
		bool needsComUninit = false;
#endif
	};

	AudioManager::AudioManager() : _impl(std::make_unique<Impl>()) {}

	AudioManager::~AudioManager() {
		Shutdown();
	}

	bool AudioManager::Init() {
		if (_impl->system) {
			return true;
		}

#ifdef _WIN32
		const HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		if (hr == S_OK) {
			_impl->needsComUninit = true;
		}
		else if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
			fmt::print(stderr, "[Audio] CoInitializeEx failed (hr=0x{:08X})\n", static_cast<unsigned>(hr));
			return false;
		}
#endif

		FMOD::Studio::System* system = nullptr;
		FMOD_RESULT result = FMOD::Studio::System::create(&system);
		if (result != FMOD_OK) {
			LogFmodError("Studio::System::create failed", result);
#ifdef _WIN32
			if (_impl->needsComUninit) {
				CoUninitialize();
				_impl->needsComUninit = false;
			}
#endif
			return false;
		}

		result = system->initialize(512, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, nullptr);
		if (result != FMOD_OK) {
			LogFmodError("Studio::System::initialize failed", result);
			system->release();
#ifdef _WIN32
			if (_impl->needsComUninit) {
				CoUninitialize();
				_impl->needsComUninit = false;
			}
#endif
			return false;
		}

		_impl->system = system;
		return true;
	}

	void AudioManager::Shutdown() {
		if (!_impl) {
			return;
		}

		if (_impl->system) {
			StopAllEvents();
			UnloadAllBanks();
			_impl->system->release();
			_impl->system = nullptr;
		}

#ifdef _WIN32
		if (_impl->needsComUninit) {
			CoUninitialize();
			_impl->needsComUninit = false;
		}
#endif
	}

	void AudioManager::Update() {
		if (!_impl->system) {
			return;
		}
		const FMOD_RESULT result = _impl->system->update();
		if (result != FMOD_OK) {
			LogFmodError("Studio::System::update failed", result);
		}
	}

	bool AudioManager::LoadBank(const std::filesystem::path& bankPath) {
		if (!_impl->system) {
			return false;
		}

		const std::filesystem::path resolved = ResolveContentPath(bankPath);
		const std::string pathUtf8 = resolved.string();

		FMOD::Studio::Bank* bank = nullptr;
		const FMOD_RESULT result = _impl->system->loadBankFile(pathUtf8.c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &bank);
		if (result == FMOD_ERR_EVENT_ALREADY_LOADED) {
			return true;
		}
		if (result != FMOD_OK) {
			LogFmodError(fmt::format("loadBankFile failed ({})", pathUtf8), result);
			return false;
		}
		_impl->banks.push_back(bank);

		if (!EndsWithStringsBank(resolved)) {
			const std::filesystem::path stringsBank =
			    resolved.parent_path() / (resolved.stem().string() + ".strings.bank");
			std::error_code ec;
			if (std::filesystem::exists(stringsBank, ec) && !ec) {
				LoadBank(stringsBank);
			}
		}

		return true;
	}

	void AudioManager::UnloadAllBanks() {
		if (!_impl->system) {
			_impl->banks.clear();
			return;
		}

		for (FMOD::Studio::Bank* bank : _impl->banks) {
			if (bank) {
				bank->unload();
			}
		}
		_impl->banks.clear();
		_impl->system->flushCommands();
	}

	bool AudioManager::PlayEvent(std::string_view eventPath) {
		return PlayEvent(eventPath, {});
	}

	bool AudioManager::PlayEvent(std::string_view eventPath, std::span<const EventParameter> parameters) {
		if (!_impl->system) {
			return false;
		}

		const std::string path(eventPath);
		FMOD::Studio::EventDescription* description = nullptr;
		FMOD_RESULT result = _impl->system->getEvent(path.c_str(), &description);
		if (result != FMOD_OK) {
			LogFmodError(fmt::format("getEvent failed ({})", path), result);
			return false;
		}

		FMOD::Studio::EventInstance* instance = nullptr;
		result = description->createInstance(&instance);
		if (result != FMOD_OK) {
			LogFmodError(fmt::format("createInstance failed ({})", path), result);
			return false;
		}

		for (const EventParameter& parameter : parameters) {
			result = instance->setParameterByName(parameter.name.data(), parameter.value);
			if (result != FMOD_OK) {
				LogFmodError(fmt::format("setParameterByName failed ({}, parameter={})", path, parameter.name), result);
			}
		}

		result = instance->start();
		if (result != FMOD_OK) {
			LogFmodError(fmt::format("EventInstance::start failed ({})", path), result);
			instance->release();
			return false;
		}

		instance->release();
		return true;
	}

	void AudioManager::StopAllEvents() {
		if (!_impl->system) {
			return;
		}

		FMOD::Studio::Bus* masterBus = nullptr;
		const FMOD_RESULT result = _impl->system->getBus("bus:/", &masterBus);
		if (result != FMOD_OK) {
			return;
		}
		masterBus->stopAllEvents(FMOD_STUDIO_STOP_IMMEDIATE);
	}

	bool AudioManager::IsInitialized() const {
		return _impl && _impl->system != nullptr;
	}

} // namespace Engine
