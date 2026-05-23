#include "Engine/Background/TiledGameBackground.h"

#include "Engine/Core/MainContext.h"
#include "Engine/Core/TextureManager.h"
#include "TiledGameBackground.generated.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

#include <algorithm>
#include <cmath>
#include <filesystem>

void TiledGameBackground::Configure(
    const std::string& texturePath, float opacity, float staticity, float defaultScale) {
	SetTexturePath(texturePath);
	SetOpacity(opacity);
	SetStaticity(staticity);
	SetDefaultScale(defaultScale);
	_haveReferenceView = false;
}

const std::string& TiledGameBackground::GetTexturePath() const {
	return _texturePath;
}

void TiledGameBackground::SetTexturePath(std::string path) {
	_texturePath = std::move(path);
	_texture.reset();

	if (_texturePath.empty()) {
		_geometryDirty = true;
		return;
	}

	auto tm = Engine::MainContext::GetInstance().GetTextureManager();
	if (!tm) {
		_geometryDirty = true;
		return;
	}

	_texture = tm->LoadTexture(std::filesystem::path(_texturePath));
	if (_texture) {
		_texture->setRepeated(true);
		_texture->setSmooth(true);
	}
	_geometryDirty = true;
}

float TiledGameBackground::GetOpacity() const {
	return _opacity;
}

void TiledGameBackground::SetOpacity(float opacity) {
	_opacity = std::clamp(opacity, 0.f, 1.f);
	_geometryDirty = true;
}

float TiledGameBackground::GetDefaultScale() const {
	return _defaultScale;
}

void TiledGameBackground::SetDefaultScale(float defaultScale) {
	_defaultScale = std::max(128.f, defaultScale);
	_geometryDirty = true;
}

float TiledGameBackground::GetStaticity() const {
	return _staticity;
}

void TiledGameBackground::SetStaticity(float staticity) {
	_staticity = staticity;
	_geometryDirty = true;
}

sf::Vector2i TiledGameBackground::GetTextureOffset() const {
	return _textureOffset;
}

void TiledGameBackground::SetTextureOffset(sf::Vector2i offset) {
	if (_textureOffset == offset) {
		return;
	}
	_textureOffset = offset;
	_geometryDirty = true;
}

namespace {
	constexpr float kMinTile = 1e-3f;

	float StablePow(float base, float exp) {
		if (base <= 0.f) {
			return 0.f;
		}
		return std::pow(base, exp);
	}
} // namespace

void TiledGameBackground::Update(const sf::RenderWindow& window, sf::Time /*dt*/) {
	const sf::View view = window.getView();
	const sf::Vector2f center = view.getCenter();
	const sf::Vector2f size = view.getSize();
	const float vw = size.x;
	if (vw > kMinTile && !_haveReferenceView) {
		_referenceViewWidth = vw;
		_haveReferenceView = true;
		_geometryDirty = true;
	}

	if (!_hasCachedBuildState || center != _cachedViewCenter || size != _cachedViewSize) {
		_geometryDirty = true;
		_hasCachedBuildState = true;
		_cachedViewCenter = center;
		_cachedViewSize = size;
	}
}

void TiledGameBackground::RebuildVertices(const sf::RenderTarget& target) const {
	_vertices.clear();
	if (!_texture) {
		return;
	}

	const sf::Vector2u texSizeU = _texture->getSize();
	if (texSizeU.x == 0u || texSizeU.y == 0u) {
		return;
	}
	const float texW = static_cast<float>(texSizeU.x);
	const float texH = static_cast<float>(texSizeU.y);

	const sf::View view = target.getView();
	const sf::Vector2f center = view.getCenter();
	const sf::Vector2f size = view.getSize();

	const float viewW = std::max(size.x, kMinTile);
	float zoomRatio = _referenceViewWidth / viewW;
	if (!_haveReferenceView || zoomRatio <= 0.f) {
		zoomRatio = 1.f;
	}

	const float scaleCam = -_staticity;
	const float moveCam = _staticity;

	const float worldTileW = std::max(kMinTile, _defaultScale * StablePow(zoomRatio, scaleCam));
	const float aspect = texH / texW;
	const float worldTileH = std::max(kMinTile, worldTileW * aspect);
	const float scrollX = center.x * moveCam;
	const float scrollY = center.y * moveCam;

	const float halfW = size.x * 0.5f;
	const float halfH = size.y * 0.5f;
	const float margin = std::max(std::max(worldTileW, worldTileH) * 2.f, std::max(size.x, size.y) * 0.5f);

	const float minX = center.x - halfW - margin;
	const float maxX = center.x + halfW + margin;
	const float minY = center.y - halfH - margin;
	const float maxY = center.y + halfH + margin;

	const std::uint8_t alpha = static_cast<std::uint8_t>(_opacity * 255.f);
	const sf::Color vertColor(255, 255, 255, alpha);
	const float texOffsetU = static_cast<float>(_textureOffset.x);
	const float texOffsetV = static_cast<float>(_textureOffset.y);

	auto addQuad = [&](float x0, float y0, float x1, float y1) {
		const float u0 = ((x0 - scrollX) / worldTileW) * texW + texOffsetU;
		const float v0 = ((y0 - scrollY) / worldTileH) * texH + texOffsetV;
		const float u1 = ((x1 - scrollX) / worldTileW) * texW + texOffsetU;
		const float v1 = ((y1 - scrollY) / worldTileH) * texH + texOffsetV;

		sf::Vertex v[6];
		v[0].position = {x0, y0};
		v[0].texCoords = {u0, v0};
		v[0].color = vertColor;
		v[1].position = {x1, y0};
		v[1].texCoords = {u1, v0};
		v[1].color = vertColor;
		v[2].position = {x0, y1};
		v[2].texCoords = {u0, v1};
		v[2].color = vertColor;
		v[3].position = {x1, y0};
		v[3].texCoords = {u1, v0};
		v[3].color = vertColor;
		v[4].position = {x1, y1};
		v[4].texCoords = {u1, v1};
		v[4].color = vertColor;
		v[5].position = {x0, y1};
		v[5].texCoords = {u0, v1};
		v[5].color = vertColor;
		for (auto& vertex : v) {
			_vertices.append(vertex);
		}
	};

	for (float x = minX; x < maxX - 0.5f * kMinTile; x += worldTileW) {
		for (float y = minY; y < maxY - 0.5f * kMinTile; y += worldTileH) {
			addQuad(x, y, x + worldTileW, y + worldTileH);
		}
	}
}

void TiledGameBackground::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (!_texture) {
		return;
	}
	if (_geometryDirty) {
		RebuildVertices(target);
		_geometryDirty = false;
	}
	if (_vertices.getVertexCount() == 0) {
		return;
	}
	states.texture = _texture.get();
	target.draw(_vertices, states);
}
