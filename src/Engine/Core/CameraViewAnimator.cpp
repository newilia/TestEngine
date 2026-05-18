#include "CameraViewAnimator.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>

#include <cmath>

namespace {

	constexpr float kSmoothingRate = 16.f;
	constexpr float kSnapEpsilonSq = 1e-6f;

	[[nodiscard]] sf::View CopyViewPresentation(const sf::View& from, sf::Vector2f center, sf::Vector2f size) {
		sf::View view(center, size);
		view.setRotation(from.getRotation());
		view.setViewport(from.getViewport());
		view.setScissor(from.getScissor());
		return view;
	}

	void ApplyViewKeepingWorldAtPixel(sf::RenderWindow& window, sf::View& view, sf::Vector2f size,
	    sf::Vector2i anchorPixel, sf::Vector2f anchorWorld) {
		view.setSize(size);
		const sf::Vector2f worldAtPixel = window.mapPixelToCoords(anchorPixel, view);
		view.move(anchorWorld - worldAtPixel);
	}

	[[nodiscard]] float SmoothingStep(float dtSec) {
		if (dtSec <= 0.f) {
			return 1.f;
		}
		return 1.f - std::exp(-kSmoothingRate * dtSec);
	}

} // namespace

namespace Engine {

	bool CameraViewAnimator::IsAnimating() const {
		return _hasTarget;
	}

	void CameraViewAnimator::Cancel() {
		_hasTarget = false;
		_anchorPixel.reset();
		_anchorWorld.reset();
	}

	void CameraViewAnimator::ApplyView(sf::RenderWindow& window, const sf::View& view) {
		window.setView(view);
	}

	sf::Vector2f CameraViewAnimator::GetBaseSizeForZoomRequest(const sf::View& currentView) const {
		return _hasTarget ? _targetSize : currentView.getSize();
	}

	void CameraViewAnimator::RequestZoom(
	    sf::RenderWindow& window, float zoomFactor, std::optional<sf::Vector2i> focusPixel) {
		const sf::View currentView = window.getView();
		_targetSize = GetBaseSizeForZoomRequest(currentView) * zoomFactor;
		if (focusPixel) {
			_anchorPixel = focusPixel;
			_anchorWorld = window.mapPixelToCoords(*focusPixel, currentView);
		}
		else {
			_targetCenter = currentView.getCenter();
			_anchorPixel.reset();
			_anchorWorld.reset();
		}
		_hasTarget = true;
	}

	void CameraViewAnimator::RequestFocusCenter(sf::RenderWindow& window, sf::Vector2f worldCenter) {
		const sf::View currentView = window.getView();
		_targetCenter = worldCenter;
		_targetSize = _hasTarget ? _targetSize : currentView.getSize();
		_anchorPixel.reset();
		_anchorWorld.reset();
		_hasTarget = true;
	}

	void CameraViewAnimator::OffsetTargetCenter(sf::Vector2f worldDelta) {
		if (!_hasTarget) {
			return;
		}
		_targetCenter += worldDelta;
		if (_anchorWorld) {
			*_anchorWorld += worldDelta;
		}
	}

	void CameraViewAnimator::ScaleTargets(sf::Vector2f sizeScale) {
		if (!_hasTarget) {
			return;
		}
		_targetSize = {_targetSize.x * sizeScale.x, _targetSize.y * sizeScale.y};
	}

	void CameraViewAnimator::Update(sf::RenderWindow& window, float dtSec) {
		if (!_hasTarget) {
			return;
		}

		const sf::View previous = window.getView();
		const float step = SmoothingStep(dtSec);

		sf::Vector2f size = previous.getSize();
		size.x += (_targetSize.x - size.x) * step;
		size.y += (_targetSize.y - size.y) * step;

		sf::View next = CopyViewPresentation(previous, previous.getCenter(), size);
		if (_anchorPixel && _anchorWorld) {
			ApplyViewKeepingWorldAtPixel(window, next, size, *_anchorPixel, *_anchorWorld);
		}
		else {
			sf::Vector2f center = previous.getCenter();
			center.x += (_targetCenter.x - center.x) * step;
			center.y += (_targetCenter.y - center.y) * step;
			next.setCenter(center);
		}

		const sf::Vector2f sizeDelta = _targetSize - next.getSize();
		float remainingSq = sizeDelta.x * sizeDelta.x + sizeDelta.y * sizeDelta.y;
		if (!_anchorPixel) {
			const sf::Vector2f centerDelta = _targetCenter - next.getCenter();
			remainingSq += centerDelta.x * centerDelta.x + centerDelta.y * centerDelta.y;
		}
		if (remainingSq <= kSnapEpsilonSq || step >= 1.f) {
			next = CopyViewPresentation(previous, previous.getCenter(), _targetSize);
			if (_anchorPixel && _anchorWorld) {
				ApplyViewKeepingWorldAtPixel(window, next, _targetSize, *_anchorPixel, *_anchorWorld);
			}
			else {
				next.setCenter(_targetCenter);
			}
			_hasTarget = false;
			_anchorPixel.reset();
			_anchorWorld.reset();
		}

		ApplyView(window, next);
	}

} // namespace Engine
