#include "BilliardGameLaunchProfile.h"

#include "Engine/Core/MainContext.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/SfmlWindowUtils.h"
#include "Engine/Editor/Editor.h"
#include "Engine/Serialization/SceneDocumentSerializer.h"
#include "Engine/Visual/RectangleShapeVisual.h"

#include <SFML/Graphics.hpp>

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
	}

	void BilliardGameLaunchProfile::OnEvent(const sf::Event& event) {}

} // namespace Billiard
