#pragma once

#include "Engine/Behaviour/Physics/PhysicsBodyBehaviour.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>

class PhysicsProcessor;

namespace sf {
	class RenderWindow;
}

namespace Engine {

	enum class FieldGridLayout
	{
		Square,
		Triangular,
	};

	class PhysicsVisualizer
	{
	public:
		void Draw(sf::RenderWindow& window) const;

		bool IsVelocityVisible() const;
		void SetVelocityVisible(bool visible);

		bool IsForceVisible() const;
		void SetForceVisible(bool visible);

		float GetVelocityScale() const;
		void SetVelocityScale(float scale);

		float GetForceScale() const;
		void SetForceScale(float scale);

		sf::Color GetVelocityColor() const;
		void SetVelocityColor(sf::Color color);

		sf::Color GetForceColor() const;
		void SetForceColor(sf::Color color);

		bool IsAttractionFieldVisible() const;
		void SetAttractionFieldVisible(bool visible);

		FieldGridLayout GetAttractionFieldGridLayout() const;
		void SetAttractionFieldGridLayout(FieldGridLayout layout);

		float GetAttractionFieldSpacingPx() const;
		void SetAttractionFieldSpacingPx(float spacingPx);

		float GetAttractionFieldArrowScale() const;
		void SetAttractionFieldArrowScale(float scale);

		/// Reference length in world space for soft length cap on field arrows; 0 disables compression.
		float GetAttractionFieldArrowLengthSoftCap() const;
		void SetAttractionFieldArrowLengthSoftCap(float worldLength);

		/// 0 = linear (ignore soft cap), 1 = full rational compression toward the soft cap.
		float GetAttractionFieldArrowLengthCompress() const;
		void SetAttractionFieldArrowLengthCompress(float amount);

		float GetAttractionFieldPaletteSpan() const;
		void SetAttractionFieldPaletteSpan(float span);

		sf::Color GetAttractionFieldPaletteWeak() const;
		void SetAttractionFieldPaletteWeak(sf::Color color);

		sf::Color GetAttractionFieldPaletteMid() const;
		void SetAttractionFieldPaletteMid(sf::Color color);

		sf::Color GetAttractionFieldPaletteStrong() const;
		void SetAttractionFieldPaletteStrong(sf::Color color);

		const PhysicsBodyBehaviour::GroupSet& GetAttractionFieldProbeGroups() const;
		void SetAttractionFieldProbeGroups(const PhysicsBodyBehaviour::GroupSet& groups);

	private:
		void DrawAttractionFieldOverlay(sf::RenderWindow& window, const PhysicsProcessor& proc) const;

		bool _isVelocityVisible = false;
		bool _isForceVisible = false;
		float _velocityScale = 0.5f;
		float _forceScale = 0.5f;
		sf::Color _velocityColor{100, 220, 120};
		sf::Color _forceColor{255, 160, 60};

		bool _isAttractionFieldVisible = false;
		FieldGridLayout _fieldGridLayout = FieldGridLayout::Triangular;
		float _fieldSpacingPx = 48.f;
		float _fieldArrowScale = 0.5f;
		float _fieldArrowLengthSoftCap = 100.f;
		float _fieldArrowLengthCompress = 1.f;
		float _fieldPaletteSpan = 20000.f;
		sf::Color _fieldPaletteWeak{100, 150, 255, 0};
		sf::Color _fieldPaletteMid{220, 220, 100, 150};
		sf::Color _fieldPaletteStrong{255, 80, 50, 200};
		PhysicsBodyBehaviour::GroupSet _fieldProbeGroups{((1ULL << PhysicsBodyBehaviour::kGroupsCount) - 1ULL)};
	};

} // namespace Engine
