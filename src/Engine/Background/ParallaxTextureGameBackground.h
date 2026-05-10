#pragma once

#include "Engine/Background/IGameBackground.h"
#include "Engine/Core/MetaClass.h"

#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

#include <memory>
#include <string>

namespace sf {
	class RenderWindow;
}

/// Seamlessly tiled texture that follows the camera with configurable scale and scroll coupling.
class ParallaxTextureGameBackground final : public IGameBackground
{
	META_CLASS()

public:
	void Update(const sf::RenderWindow& window, sf::Time dt) override;
	void Configure(
	    const std::string& texturePath, float opacity, float scaleWithCamera, float moveWithCamera, float defaultScale);

public:
	/// @getter
	/// @valuesProvider(GetBackgroundTextures)
	const std::string& GetTexturePath() const;
	/// @setter
	void SetTexturePath(std::string path);

private:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	void RebuildVertices(const sf::RenderTarget& target) const;

private:
	std::string _texturePath;
	/// @property(minValue=0.f, maxValue=1.f, dragSpeed=0.005f)
	float _opacity = 1.f;
	/// @property(minValue=-2.f, maxValue=2.f, dragSpeed=0.005f)
	float _scaleWithCamera = 0.f;
	/// @property(minValue=-2.f, maxValue=2.f, dragSpeed=0.005f)
	float _moveWithCamera = 0.35f;
	/// @property(minValue=128f, maxValue=1.e6f, dragSpeed=4.f)
	float _defaultScale = 256.f;

private:
	std::shared_ptr<sf::Texture> _texture;
	bool _haveReferenceView = false;
	float _referenceViewWidth = 1.f;
	mutable sf::VertexArray _vertices{sf::PrimitiveType::Triangles};
	mutable bool _geometryDirty = true;
};
