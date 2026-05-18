#pragma once
#include "Engine/Background/GameBackgroundContext.h"
#include "Engine/Core/CameraViewAnimator.h"
#include "Engine/Core/Scene.h"
#include "Engine/Core/Singleton.h"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>

class PhysicsProcessor;
class FontManager;
class TextureManager;

namespace Engine::Serialization {
	enum class SceneDocumentKind;
}

namespace Engine {
	class EventsDispatcher;

	class MainContext : public Singleton<MainContext>
	{
		friend class Singleton<MainContext>;

	public:
		void Init();
		void Shutdown();
		bool IsImGuiInitialized() const;

		shared_ptr<Scene> GetScene() const;
		void SetScene(const shared_ptr<Scene>& scene);
		shared_ptr<EventsDispatcher> GetEventsDispatcher() const;
		shared_ptr<PhysicsProcessor> GetPhysicsProcessor() const;
		shared_ptr<FontManager> GetFontManager() const;
		shared_ptr<TextureManager> GetTextureManager() const;
		shared_ptr<GameBackgroundContext> GetGameBackgroundContext() const;

		std::shared_ptr<sf::RenderWindow> CreateMainWindow(sf::VideoMode mode, const sf::String& title,
		    std::uint32_t style = sf::Style::Default, sf::State state = sf::State::Windowed);

		sf::RenderWindow* GetMainWindow() const;

		void UpdateMainWindowTitle(const std::optional<std::filesystem::path>& documentFilePath,
		    std::optional<Serialization::SceneDocumentKind> documentKind);

		float GetSimSpeedMultiplier() const;
		void SetSimSpeedMultiplier(float val);

		bool IsSimPaused() const;
		void SetSimPaused(bool paused);
		void ToggleSimPaused();

		void OnStartPresentFrame();
		void OnStartUpdateTick();

		// in-game dt for logic update, affected by speed multiplier
		sf::Time GetSimTickDt() const;
		// wall-clock dt for logic update
		sf::Time GetWallTickDt() const;
		// wall-clock dt for frame presentation
		sf::Time GetFrameDt() const;

		void SetVerticalSyncEnabled(bool enabled);
		bool IsVerticalSyncEnabled() const;

		float GetCurrentFps() const;
		float GetCurrentTickRate() const;

		void MoveCamera(const sf::Vector2i& delta);
		void ZoomCamera(float zoomFactor, std::optional<sf::Vector2i> focusPixel = std::nullopt, bool smooth = true);
		void FocusCameraOnNode(const std::shared_ptr<SceneNode>& node, bool smooth = true);
		void FocusCameraOnWorldPoint(const sf::Vector2f& worldPoint, bool smooth = true);

		std::optional<sf::Vector2f> GetMainCameraCenter() const;
		std::optional<sf::Vector2f> GetMainCameraViewSize() const;
		void SetMainCameraView(sf::Vector2f center, sf::Vector2f viewSize);
		void ResetMainCameraViewToDefault();

		void OnMainWindowResized(const sf::Vector2u& newPixelSize);

	private:
		MainContext();

		void ApplyWindowFrameSettings();

	private:
		shared_ptr<sf::RenderWindow> _mainWindow;
		shared_ptr<Scene> _scene;
		shared_ptr<EventsDispatcher> _eventsDispatcher;
		shared_ptr<PhysicsProcessor> _physicsProcessor;
		shared_ptr<FontManager> _fontManager;
		shared_ptr<TextureManager> _textureManager;
		shared_ptr<GameBackgroundContext> _gameBackgroundContext;
		sf::Clock _frameClock;
		sf::Time _frameTime;
		sf::Clock _tickClock;
		sf::Time _tickTime;
		float _simSpeedMultiplier = 1.f;
		bool _isSimPaused = false;
		bool _isImGuiInitialized = false;
		bool _verticalSyncEnabled = false;
		float _tickRate = 0.f;
		bool _haveTickRate = false;
		sf::Vector2u _mainWindowPixelSizeForView{};
		bool _haveMainWindowPixelSizeForView = false;
		std::string _mainWindowBaseTitleUtf8;
		CameraViewAnimator _cameraViewAnimator;
	};
} // namespace Engine
