#include "MainContext.h"

#include <imgui-SFML.h>
#include <imgui.h>

#include <cmath>

namespace {
	constexpr float kRateSmoothing = 0.92f;

	void AccumulateSmoothedRate(float instantaneous, float& smoothed, bool& isValid) {
		if (!std::isfinite(instantaneous) || instantaneous <= 0.f) {
			return;
		}
		if (!isValid) {
			smoothed = instantaneous;
			isValid = true;
			return;
		}
		smoothed = smoothed * kRateSmoothing + instantaneous * (1.f - kRateSmoothing);
	}
} // namespace

namespace Engine {

	shared_ptr<Scene> MainContext::GetScene() const {
		return _scene;
	}

	shared_ptr<UserInput> MainContext::GetUserInput() const {
		return _userInput;
	}

	shared_ptr<PhysicsProcessor> MainContext::GetPhysicsProcessor() const {
		return _physicsProcessor;
	}

	shared_ptr<FontManager> MainContext::GetFontManager() const {
		return _fontManager;
	}

	float MainContext::GetSimSpeedMultiplier() const {
		return _simSpeedMultiplier;
	}

	void MainContext::SetSimSpeedMultiplier(float val) {
		_simSpeedMultiplier = val;
	}

	bool MainContext::IsSimPaused() const {
		return _isSimPaused;
	}

	void MainContext::SetSimPaused(bool paused) {
		_isSimPaused = paused;
	}

	void MainContext::ToggleSimPaused() {
		SetSimPaused(!_isSimPaused);
	}

	sf::RenderWindow* MainContext::GetMainWindow() const {
		return _mainWindow.get();
	}

	bool MainContext::IsDebugDrawEnabled() const {
		return _isDebugDrawEnabled;
	}

	void MainContext::SetDebugDrawEnabled(bool enabled) {
		_isDebugDrawEnabled = enabled;
	}

	float MainContext::GetFieldForceDebugArrowScale() const {
		return _fieldForceDebugArrowScale;
	}

	void MainContext::SetFieldForceDebugArrowScale(float scale) {
		_fieldForceDebugArrowScale = scale >= 0.f ? scale : 0.f;
	}

	std::uint32_t MainContext::GetFramerateLimit() const {
		return _framerateLimit;
	}

	std::uint32_t MainContext::GetTargetTickRate() const {
		return _targetTickRateHz;
	}

	void MainContext::SetTargetTickRate(std::uint32_t hz) {
		_targetTickRateHz = hz;
	}

	void MainContext::SetVerticalSyncEnabled(bool enabled) {
		_verticalSyncEnabled = enabled;
		ApplyWindowFrameSettings();
	}

	bool MainContext::IsVerticalSyncEnabled() const {
		return _verticalSyncEnabled;
	}

	MainContext::MainContext() {
		_userInput = make_shared<UserInput>();
		_physicsProcessor = make_shared<PhysicsProcessor>();
		_fontManager = make_shared<FontManager>();
	}

	MainContext::~MainContext() {}

	void MainContext::SetScene(const shared_ptr<Scene>& scene) {
		if (_scene) {
			_scene->NotifyLifecycleDeinitRecursive();
		}
		_scene = scene;
		if (_scene) {
			_scene->NotifyLifecycleInitRecursive();
		}
	}

	void MainContext::Init() {
		if (!_isImGuiInitialized) {
			if (!ImGui::SFML::Init(*_mainWindow)) {
				return;
			}
			ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
			_isImGuiInitialized = true;
		}
	}

	void MainContext::Shutdown() {
		SetScene(nullptr); // TODO is necessary?

		if (_isImGuiInitialized) {
			ImGui::SFML::Shutdown();
		}
	}

	bool MainContext::IsImGuiInitialized() const {
		return _isImGuiInitialized;
	}

	sf::Time MainContext::GetSimTickDt() const {
		if (_isSimPaused) {
			return sf::Time();
		}
		return _tickTime * _simSpeedMultiplier;
	}

	sf::Time MainContext::GetWallTickDt() const {
		return _tickTime;
	}

	sf::Time MainContext::GetFrameDt() const {
		return _frameTime;
	}

	void MainContext::OnStartPresentFrame() {
		_frameTime = _frameClock.getElapsedTime();
		_frameClock.restart();
		const float dtSec = _frameTime.asSeconds();
		if (dtSec > 0.f) {
			const float instantaneous = 1.f / dtSec;
		}
	}

	void MainContext::OnStartUpdateTick() {
		_tickTime = _tickClock.getElapsedTime();
		_tickClock.restart();
		const float dtSec = _tickTime.asSeconds();
		if (dtSec > 0.f) {
			const float instantaneous = 1.f / dtSec;
			AccumulateSmoothedRate(instantaneous, _tickRate, _haveTickRate);
		}
	}

	void MainContext::ApplyWindowFrameSettings() {
		if (!_mainWindow) {
			return;
		}
		_mainWindow->setVerticalSyncEnabled(_verticalSyncEnabled);
	}

	std::shared_ptr<sf::RenderWindow> MainContext::CreateMainWindow(sf::VideoMode mode, const sf::String& title,
	                                                                std::uint32_t style, sf::State state) {
		_mainWindow = std::make_shared<sf::RenderWindow>(mode, title, style, state);
		ApplyWindowFrameSettings();
		_mainWindowPixelSizeForView = _mainWindow->getSize();
		_haveMainWindowPixelSizeForView = _mainWindowPixelSizeForView.x != 0u && _mainWindowPixelSizeForView.y != 0u;
		return _mainWindow;
	}

	void MainContext::SetFramerateLimit(std::uint32_t maxFps) {
		_framerateLimit = maxFps;
	}

	void MainContext::SetFramerateLimitEnabled(bool enabled) {
		_isFramerateLimitEnabled = enabled;
	}

	bool MainContext::IsFramerateLimitEnabled() const {
		return _isFramerateLimitEnabled;
	}

	float MainContext::GetCurrentFps() const {
		return ImGui::GetIO().Framerate;
	}

	float MainContext::GetCurrentTickRate() const {
		return _haveTickRate ? _tickRate : 0.f;
	}

	void MainContext::SetHierarchySelectedForViewport(const shared_ptr<Scene>& node) {
		_hierarchySelectedForViewport = node;
	}

	shared_ptr<Scene> MainContext::GetHierarchySelectedForViewport() const {
		return _hierarchySelectedForViewport.lock();
	}

	void MainContext::MoveCamera(const sf::Vector2i& delta) {
		if (auto window = GetMainWindow()) {
			auto view = window->getView();
			view.move(static_cast<sf::Vector2f>(delta));
			window->setView(view);
		}
	}

	void MainContext::ZoomCamera(float zoomFactor, std::optional<sf::Vector2i> focusPixel) {
		if (auto window = GetMainWindow()) {
			auto view = window->getView();
			if (focusPixel) {
				const sf::Vector2f worldBefore = window->mapPixelToCoords(*focusPixel, view);
				view.zoom(zoomFactor);
				const sf::Vector2f worldAfter = window->mapPixelToCoords(*focusPixel, view);
				view.move(worldBefore - worldAfter);
			}
			else {
				view.zoom(zoomFactor);
			}
			window->setView(view);
		}
	}

	void MainContext::OnMainWindowResized(const sf::Vector2u& newPixelSize) {
		if (!_mainWindow || newPixelSize.x == 0u || newPixelSize.y == 0u) {
			return;
		}
		auto view = _mainWindow->getView();
		const sf::Vector2f center = view.getCenter();
		const sf::Vector2f viewSize = view.getSize();

		sf::Vector2f adjustedSize = viewSize;
		if (_haveMainWindowPixelSizeForView && _mainWindowPixelSizeForView.x != 0u &&
		    _mainWindowPixelSizeForView.y != 0u) {
			const float sx = static_cast<float>(newPixelSize.x) / static_cast<float>(_mainWindowPixelSizeForView.x);
			const float sy = static_cast<float>(newPixelSize.y) / static_cast<float>(_mainWindowPixelSizeForView.y);
			adjustedSize = {viewSize.x * sx, viewSize.y * sy};
		}

		sf::View adjusted(center, adjustedSize);
		adjusted.setRotation(view.getRotation());
		adjusted.setViewport(view.getViewport());
		adjusted.setScissor(view.getScissor());
		_mainWindow->setView(adjusted);

		_mainWindowPixelSizeForView = newPixelSize;
		_haveMainWindowPixelSizeForView = true;
	}

} // namespace Engine
