#include "Engine/Visual/ComposedSurfaceDraw.h"

#include "Engine/Behaviour/ComposedSurface/ComposedSurfaceContributorBehaviour.h"
#include "Engine/Behaviour/ComposedSurface/ComposedSurfaceTypes.h"
#include "Engine/Behaviour/ComposedSurface/SphereProjectionContributorBehaviour.h"
#include "Engine/Behaviour/ComposedSurface/TiledTextureContributorBehaviour.h"
#include "Engine/Core/RefWrapper.h"
#include "Engine/Visual/ComposedSurfaceVisual.h"

#include <SFML/Graphics/Glsl.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <string>

namespace Engine {

	namespace {

		struct ResolvedContributors
		{
			const ComposedSurfaceContributorBehaviour* tile = nullptr;
			const ComposedSurfaceContributorBehaviour* sphereProjection = nullptr;
		};

		ResolvedContributors ResolveContributors(
		    const std::vector<RefWrapper<ComposedSurfaceContributorBehaviour>>& contributors) {
			ResolvedContributors resolved{};
			for (const RefWrapper<ComposedSurfaceContributorBehaviour>& ref : contributors) {
				const auto contributor = ref.Get();
				if (!contributor || !contributor->IsContributorEnabled()) {
					continue;
				}
				switch (contributor->GetContributorKind()) {
				case ComposedSurfaceContributorKind::Tile:
					if (!resolved.tile) {
						resolved.tile = contributor.get();
					}
					break;
				case ComposedSurfaceContributorKind::SphereProjection:
					if (!resolved.sphereProjection) {
						resolved.sphereProjection = contributor.get();
					}
					break;
				}
			}
			return resolved;
		}

		sf::Shader* GetComposedSurfaceShader() {
			static sf::Shader shader;
			static bool tried = false;
			static bool ok = false;
			if (!tried) {
				tried = true;
				if (sf::Shader::isAvailable()) {
					ok = shader.loadFromFile("assets/shaders/ComposedSurface.frag", sf::Shader::Type::Fragment);
				}
			}
			return ok ? &shader : nullptr;
		}

	} // namespace

	void DrawComposedSurface(const ComposedSurfaceVisual& visual, sf::RenderTarget& target, sf::RenderStates states) {
		const ResolvedContributors resolved = ResolveContributors(visual.GetContributors());
		if (!resolved.tile) {
			return;
		}

		ComposedSurfaceTileData tile{};
		if (!resolved.tile->TryContributeTile(tile) || !tile.texture) {
			return;
		}

		ComposedSurfaceSphereProjectionData sphereProjection{};
		if (resolved.sphereProjection) {
			(void)resolved.sphereProjection->TryContributeSphereProjection(sphereProjection);
		}

		const sf::Vector2f size = visual.GetSize();
		if (size.x <= 0.f || size.y <= 0.f) {
			return;
		}

		sf::Shader* shader = GetComposedSurfaceShader();
		if (!shader) {
			return;
		}

		sf::RectangleShape quad(size);
		quad.setOrigin(visual.GetOrigin());
		quad.setPosition({0.f, 0.f});
		quad.setTexture(tile.texture);
		quad.setTextureRect(sf::IntRect({0, 0},
		    sf::Vector2i(static_cast<int>(tile.texture->getSize().x), static_cast<int>(tile.texture->getSize().y))));

		const sf::Transform worldFromLocal = states.transform * quad.getTransform();
		const sf::Glsl::Mat3 localFromWorld = worldFromLocal.getInverse();

		const sf::Vector2u targetSize = target.getSize();
		const sf::Vector2f w00 = target.mapPixelToCoords({0, 0});
		const sf::Vector2f wx = target.mapPixelToCoords({1, 0});
		const sf::Vector2f wy = target.mapPixelToCoords({0, 1});

		const sf::Glsl::Vec4 tint(static_cast<float>(tile.tint.r) / 255.f, static_cast<float>(tile.tint.g) / 255.f,
		    static_cast<float>(tile.tint.b) / 255.f, static_cast<float>(tile.tint.a) / 255.f);

		shader->setUniform("u_bounds_size", sf::Glsl::Vec2(size.x, size.y));
		shader->setUniform("u_local_from_world", localFromWorld);
		shader->setUniform("u_tiling", sf::Glsl::Vec2(tile.tiling.x, tile.tiling.y));
		shader->setUniform("u_uv_offset", sf::Glsl::Vec2(tile.uvOffset.x, tile.uvOffset.y));
		shader->setUniform("u_tint", tint);
		shader->setUniform("u_sphere_projection_active", sphereProjection.active ? 1 : 0);
		shader->setUniform(
		    "u_sphere_uv_offset", sf::Glsl::Vec2(sphereProjection.sphereUvOffset.x, sphereProjection.sphereUvOffset.y));
		shader->setUniform("u_sphere_unwrap", sphereProjection.sphereUnwrap);
		shader->setUniform("u_sphere_orientation",
		    sf::Glsl::Vec4(sphereProjection.sphereOrientationX, sphereProjection.sphereOrientationY,
		        sphereProjection.sphereOrientationZ, sphereProjection.sphereOrientationW));
		shader->setUniform("u_world_origin", sf::Glsl::Vec2(w00.x, w00.y));
		shader->setUniform("u_world_dx", sf::Glsl::Vec2(wx.x - w00.x, wx.y - w00.y));
		shader->setUniform("u_world_dy", sf::Glsl::Vec2(wy.x - w00.x, wy.y - w00.y));
		shader->setUniform("u_target_height", static_cast<float>(targetSize.y));
		shader->setUniform("texture", sf::Shader::CurrentTexture);

		sf::RenderStates drawStates = states;
		drawStates.shader = shader;
		target.draw(quad, drawStates);
	}

} // namespace Engine
