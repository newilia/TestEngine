#pragma once

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/VertexArray.hpp>

namespace sf {
	class RenderTarget;
	struct RenderStates;
} // namespace sf

class CircleShape : public sf::CircleShape
{
public:
	CircleShape();
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	void setRadius(float radius);
	bool GetDrawSector() const;
	void SetDrawSector(bool drawSector);
	sf::Color GetSectorColor() const;
	void SetSectorColor(sf::Color color);

private:
	void DrawSector(sf::RenderTarget& target, sf::RenderStates states) const;
	void RebuildSectorVertices() const;

	bool _drawSector = true;
	sf::Color _sectorColor;
	mutable sf::VertexArray _sectorVertices;
	mutable bool _isSectorDirty = true;
};
