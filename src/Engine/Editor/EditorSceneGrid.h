#pragma once

#include <SFML/System/Vector2.hpp>

namespace sf {
	class RenderWindow;
}

namespace Engine {

	class EditorSceneGrid
	{
	public:
		static constexpr int kMinSize = 10;
		static constexpr int kMaxSize = 512;
		static constexpr int kMinBasis = 2;
		static constexpr int kMaxBasis = 10;

		EditorSceneGrid();

		void Draw(sf::RenderWindow& window) const;
		void ResetToDefaults();

		bool IsVisible() const;
		bool& VisibleMutable();

		bool IsSnapEnabled() const;
		bool& SnapEnabledMutable();

		int GetSize() const;
		int& SizeMutable();

		int GetBasis() const;
		int& BasisMutable();

		sf::Vector2f SnapWorldPoint(sf::Vector2f world) const;

	private:
		float GetFinestStep() const;
		float GetFinestStepAtLod(int lodLevel) const;
		int ComputeLodLevel(float pixelsPerWorld) const;
		float GetSnapStep(float pixelsPerWorld) const;

		bool _isVisible = false;
		bool _snapToGrid = false;
		int _size = 0;
		int _basis = 0;
	};

} // namespace Engine
