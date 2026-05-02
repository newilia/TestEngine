#include "Engine/App/MainContext.h"
#include "Engine/App/MainLoop.h"
#include "Engine/App/UserInput.h"
#include "Engine/Core/PeriodicTaskExecutor.h"
#include "Engine/Editor/Editor.h"
#include "Environments/Pong/PongEnvironment.h"
#include "Environments/Test/TestEnvironment.h"

#include <SFML/Graphics.hpp>

#include <fmt/format.h>
#include <imgui-SFML.h>
#include <imgui.h>

#include <cctype>
#include <string_view>

namespace {

	enum class AppEnvironmentKind
	{
		Test,
		Pong
	};

	bool EqualsIgnoreCaseAscii(std::string_view a, std::string_view b) {
		if (a.size() != b.size()) {
			return false;
		}
		for (size_t i = 0; i < a.size(); ++i) {
			if (std::tolower(static_cast<unsigned char>(a[i])) != std::tolower(static_cast<unsigned char>(b[i]))) {
				return false;
			}
		}
		return true;
	}

	[[nodiscard]] AppEnvironmentKind ParseEnvFromArgv(int argc, char** argv) {
		constexpr std::string_view prefix = "--env=";
		for (int i = 1; i < argc; ++i) {
			std::string_view arg(argv[i]);
			if (arg.size() >= prefix.size() && arg.substr(0, prefix.size()) == prefix) {
				const std::string_view value = arg.substr(prefix.size());
				if (EqualsIgnoreCaseAscii(value, "pong")) {
					return AppEnvironmentKind::Pong;
				}
				if (EqualsIgnoreCaseAscii(value, "test")) {
					return AppEnvironmentKind::Test;
				}
			}
		}
		return AppEnvironmentKind::Test;
	}

	[[nodiscard]] AppEnvironmentKind ParseEnvFromCmdLine(std::string_view fullLine) {
		constexpr std::string_view key = "--env=";
		size_t pos = 0;
		while (pos < fullLine.size()) {
			const size_t found = fullLine.find(key, pos);
			if (found == std::string_view::npos) {
				break;
			}
			size_t valStart = found + key.size();
			size_t valEnd = valStart;
			while (valEnd < fullLine.size() && !std::isspace(static_cast<unsigned char>(fullLine[valEnd]))) {
				++valEnd;
			}
			const std::string_view value = fullLine.substr(valStart, valEnd - valStart);
			if (EqualsIgnoreCaseAscii(value, "pong")) {
				return AppEnvironmentKind::Pong;
			}
			if (EqualsIgnoreCaseAscii(value, "test")) {
				return AppEnvironmentKind::Test;
			}
			pos = valEnd;
		}
		return AppEnvironmentKind::Test;
	}

	void RunAppEnvironment(AppEnvironmentKind kind) {
		switch (kind) {
		case AppEnvironmentKind::Test:
			TestEnvironment::Setup();
			return;
		case AppEnvironmentKind::Pong: {
			PongEnvironment pong;
			pong.Setup();
			return;
		}
		}
	}
} // namespace

#ifdef _CONSOLE
int main(int argc, char** argv) {
#else
#define NOMINMAX
#include <windows.h>
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine,
                   _In_ int nShowCmd) {
#endif
	_set_error_mode(_OUT_TO_MSGBOX);

#ifdef _CONSOLE
	RunAppEnvironment(ParseEnvFromArgv(argc, argv));
#else
	RunAppEnvironment(ParseEnvFromCmdLine(lpCmdLine ? std::string_view(lpCmdLine) : std::string_view{}));
#endif

	Engine::MainContext::GetInstance().Init();

	Engine::MainLoop mainLoop;
	mainLoop.Run();

	Engine::MainContext::GetInstance().Shutdown();

	return EXIT_SUCCESS;
}
