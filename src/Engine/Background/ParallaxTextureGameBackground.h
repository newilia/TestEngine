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
	void Configure(const std::string& texturePath, float opacity, float staticity, float defaultScale);

public:
	/// @getter
	/// @valuesProvider(GetBackgroundTextures)
	const std::string& GetTexturePath() const;
	/// @setter
	void SetTexturePath(std::string path);

	/// @getter(minValue=0.f, maxValue=1.f, dragSpeed=0.005f)
	float GetOpacity() const;
	/// @setter
	void SetOpacity(float opacity);
	/// @getter(minValue=128f, maxValue=1.e6f, dragSpeed=4.f)
	float GetDefaultScale() const;
	/// @setter
	void SetDefaultScale(float defaultScale);
	/// @getter(minValue=0.f, maxValue=1.f, dragSpeed=0.005f)
	float GetStaticity() const;
	/// @setter
	void SetStaticity(float staticity);

private:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	void RebuildVertices(const sf::RenderTarget& target) const;

private:
	std::string _texturePath;
	float _opacity = 1.f;
	float _defaultScale = 256.f;
	float _staticity = 0.f;

private:
	std::shared_ptr<sf::Texture> _texture;
	bool _haveReferenceView = false;
	float _referenceViewWidth = 1.f;
	bool _hasCachedBuildState = false;
	sf::Vector2f _cachedViewCenter{};
	sf::Vector2f _cachedViewSize{};
	mutable sf::VertexArray _vertices{sf::PrimitiveType::Triangles};
	mutable bool _geometryDirty = true;
};
