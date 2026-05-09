#include "ShapeLightingDraw.h"

#include "Engine/Behaviour/ShapeLightReceiverBehaviour.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Utils.h"
#include "Engine/Render/SceneLighting.h"
#include "Engine/Visual/ShapeVisualBase.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/Glsl.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Shape.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <vector>

namespace Engine {

	namespace {

		constexpr std::size_t kMaxLights = 64;
		constexpr std::size_t kMaxConvexVerts = 64;

		sf::Shader* GetShapeLightingShader() {
			static sf::Shader shader;
			static bool tried = false;
			static bool ok = false;
			if (!tried) {
				tried = true;
				if (sf::Shader::isAvailable()) {
					ok = shader.loadFromFile("assets/shaders/ShapeLighting.frag", sf::Shader::Type::Fragment);
				}
			}
			return ok ? &shader : nullptr;
		}

	} // namespace

	bool TryDrawShapeWithLighting(const ShapeVisualBase& visual, sf::RenderTarget& target, sf::RenderStates states) {
		if (!SceneLighting::GetInstance().IsEnabled()) {
			return false;
		}

		const auto node = visual.GetNode();
		if (!node) {
			return false;
		}
		const auto recv = node->FindBehaviour<ShapeLightReceiverBehaviour>();
		if (!recv || !recv->IsReceiverLightingEnabled()) {
			return false;
		}
		sf::Shader* shader = GetShapeLightingShader();
		if (!shader) {
			return false;
		}
		const sf::Shape* shape = visual.GetBaseShape();
		if (!shape) {
			return false;
		}

		const sf::Transform full = states.transform * shape->getTransform();
		const sf::FloatRect worldBounds = full.transformRect(shape->getLocalBounds());

		std::vector<Engine::GpuPointLight> lights;
		SceneLighting::GetInstance().SelectLightsForBounds(worldBounds, lights, kMaxLights);

		const sf::Transform invWorldFromLocal = full.getInverse();
		const sf::Glsl::Mat3 localFromWorld = invWorldFromLocal;

		const sf::Vector2u targetSize = target.getSize();
		const sf::Vector2f w00 = target.mapPixelToCoords({0, 0});
		const sf::Vector2f wx = target.mapPixelToCoords({1, 0});
		const sf::Vector2f wy = target.mapPixelToCoords({0, 1});

		std::array<sf::Glsl::Vec2, kMaxLights> lightPos{};
		std::array<sf::Glsl::Vec3, kMaxLights> lightCol{};
		std::array<float, kMaxLights> lightRad{};

		const std::size_t n = lights.size();
		for (std::size_t i = 0; i < n; ++i) {
			lightPos[i] = sf::Glsl::Vec2(lights[i].position.x, lights[i].position.y);
			lightCol[i] = lights[i].color;
			lightRad[i] = lights[i].radius;
		}

		const sf::Color fill = visual.GetFillColor();
		const sf::Glsl::Vec4 fillV(static_cast<float>(fill.r) / 255.f, static_cast<float>(fill.g) / 255.f,
		    static_cast<float>(fill.b) / 255.f, static_cast<float>(fill.a) / 255.f);

		int shapeKind = 2;
		int vertCount = 0;
		std::array<sf::Glsl::Vec2, kMaxConvexVerts> verts{};
		sf::Glsl::Vec2 circleCenter(0.f, 0.f);
		float circleRadius = 1.f;
		sf::Glsl::Vec2 rectMin(0.f, 0.f);
		sf::Glsl::Vec2 rectMax(1.f, 1.f);

		if (const auto* convex = dynamic_cast<const sf::ConvexShape*>(shape)) {
			shapeKind = 0;
			const unsigned pc = convex->getPointCount();
			vertCount = static_cast<int>(std::min<std::size_t>(pc, kMaxConvexVerts));
			for (int i = 0; i < vertCount; ++i) {
				const sf::Vector2f p = convex->getPoint(static_cast<unsigned>(i));
				verts[static_cast<std::size_t>(i)] = sf::Glsl::Vec2(p.x, p.y);
			}
		}
		else if (const auto* circle = dynamic_cast<const sf::CircleShape*>(shape)) {
			shapeKind = 1;
			const sf::Vector2f o = circle->getOrigin();
			circleCenter = sf::Glsl::Vec2(o.x, o.y);
			circleRadius = circle->getRadius();
		}
		else if (const auto* rect = dynamic_cast<const sf::RectangleShape*>(shape)) {
			shapeKind = 2;
			const sf::FloatRect lb = rect->getLocalBounds();
			rectMin = sf::Glsl::Vec2(lb.position.x, lb.position.y);
			rectMax = sf::Glsl::Vec2(lb.position.x + lb.size.x, lb.position.y + lb.size.y);
		}
		else {
			const sf::FloatRect lb = shape->getLocalBounds();
			rectMin = sf::Glsl::Vec2(lb.position.x, lb.position.y);
			rectMax = sf::Glsl::Vec2(lb.position.x + lb.size.x, lb.position.y + lb.size.y);
		}

		shader->setUniform("u_fill_color", fillV);
		shader->setUniform("u_world_origin", sf::Glsl::Vec2(w00.x, w00.y));
		shader->setUniform("u_world_dx", sf::Glsl::Vec2(wx.x - w00.x, wx.y - w00.y));
		shader->setUniform("u_world_dy", sf::Glsl::Vec2(wy.x - w00.x, wy.y - w00.y));
		shader->setUniform("u_target_height", static_cast<float>(targetSize.y));
		shader->setUniform("u_local_from_world", localFromWorld);
		shader->setUniform("u_shape_kind", shapeKind);
		shader->setUniform("u_vertex_count", vertCount);
		shader->setUniformArray("u_vertices", verts.data(), kMaxConvexVerts);
		shader->setUniform("u_circle_center", circleCenter);
		shader->setUniform("u_circle_radius", circleRadius);
		shader->setUniform("u_rect_min", rectMin);
		shader->setUniform("u_rect_max", rectMax);

		shader->setUniform("u_light_count", static_cast<int>(n));
		if (n > 0) {
			shader->setUniformArray("u_light_pos", lightPos.data(), n);
			shader->setUniformArray("u_light_color", lightCol.data(), n);
			shader->setUniformArray("u_light_radius", lightRad.data(), n);
		}

		shader->setUniform("u_mode_bevel", recv->IsBevelEmbossMode() ? 1 : 0);
		shader->setUniform("u_bevel_width", recv->GetBevelWidth());
		shader->setUniform("u_ease_circ", recv->IsEaseInCirc() ? 1 : 0);
		shader->setUniform("u_diffusion", recv->GetDiffusion());
		shader->setUniform("u_lighting_strength", recv->GetLightingStrength());
		shader->setUniform("u_blend_mode", static_cast<int>(SceneLighting::GetInstance().GetBlendMode()));

		states.shader = shader;
		target.draw(*shape, states);
		return true;
	}

} // namespace Engine
