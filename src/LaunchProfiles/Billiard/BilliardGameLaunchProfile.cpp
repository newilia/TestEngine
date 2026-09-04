#include "BilliardGameLaunchProfile.h"

#include "Engine/Audio/AudioManager.h"
#include "Engine/Core/MainContext.h"
#include "Engine/Core/SfmlWindowUtils.h"
#include "Engine/Editor/Editor.h"
#include "LaunchProfiles/Billiard/EightBallPoolController.h"

#include <SFML/Graphics.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <cstdlib>

namespace Billiard {

	namespace {
		constexpr const char kBilliardSceneRelativePath[] = "assets/scenes/8 ball pool.xml";
	} // namespace

	void BilliardGameLaunchProfile::Setup() {
		EventHandlerBase::SubscribeForEvents();

		auto& mainContext = Engine::MainContext::GetInstance();
		const auto mainWindow = mainContext.CreateMainWindow(sf::VideoMode::getDesktopMode(), "8 Ball Pool");
		if (!mainWindow) {
			std::exit(EXIT_FAILURE);
		}
		mainContext.Init();
		if (auto audio = mainContext.GetAudioManager()) {
			audio->LoadBank("fmod/Billiards/Build/Desktop/Master.bank");
		}
		Utils::MaximizeWindow(*mainWindow);

		auto& editor = Engine::Editor::GetInstance();
		editor.Init();

		Subscribe(editor.GetOnIsOpenChangedSignal(), [this](bool isOpen) {
			auto& editor = Engine::Editor::GetInstance();
			editor.SetCameraPanEnabled(isOpen);
			editor.SetCameraZoomEnabled(isOpen);
		});
		editor.SetIsOpen(false);

		Engine::Editor::GetInstance().LoadScene(kBilliardSceneRelativePath);
		mainContext.SetSimPaused(false);
	}

	void BilliardGameLaunchProfile::OnEvent(const sf::Event& event) {
		if (auto keyPressedEvent = event.getIf<sf::Event::KeyPressed>()) {
			if (keyPressedEvent->code == sf::Keyboard::Key::H) {
				if (auto controller = Engine::MainContext::GetInstance()
				        .GetScene()
				        ->GetRoot()
				        ->FindBehaviourRec<EightBallPoolController>()) {
					controller->StartHotSeatGame();
				}
			}
			return;
		}
	}

} // namespace Billiard
