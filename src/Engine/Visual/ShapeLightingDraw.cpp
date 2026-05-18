#include "ShapeLightingDraw.h"

#include "Engine/Behaviour/ShapeLightReceiverBehaviour.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Render/SceneLighting.h"
#include "Engine/Visual/ShapeVisualBase.h"

#include <SFML/Graphics/BlendMode.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/Glsl.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/Graphics/Shape.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <string>
#include <vector>

namespace Engine {

	namespace {

		constexpr std::size_t kMaxLights = 64;
		constexpr std::size_t kMaxConvexVerts = 64;

		const std::string kUFillColor{"u_fill_color"};
		const std::string kUWorldOrigin{"u_world_origin"};
		const std::string kUWorldDx{"u_world_dx"};
		const std::string kUWorldDy{"u_world_dy"};
		const std::string kUTargetHeight{"u_target_height"};
		const std::string kULocalFromWorld{"u_local_from_world"};
		const std::string kUShapeKind{"u_shape_kind"};
		const std::string kUVertexCount{"u_vertex_count"};
		const std::string kUVertices{"u_vertices"};
		const std::string kUCircleCenter{"u_circle_center"};
		const std::string kUCircleRadius{"u_circle_radius"};
		const std::string kURectMin{"u_rect_min"};
		const std::string kURectMax{"u_rect_max"};
		const std::string kULightCount{"u_light_count"};
		const std::string kULightPos{"u_light_pos"};
		const std::string kULightColor{"u_light_color"};
		const std::string kULightRadius{"u_light_radius"};
		const std::string kUModeBevel{"u_mode_bevel"};
		const std::string kUBevelWidth{"u_bevel_width"};
		const std::string kUEaseCirc{"u_ease_circ"};
		const std::string kUDiffusion{"u_diffusion"};
		const std::string kULightingStrength{"u_lighting_strength"};

		struct CameraUniformState
		{
			const void* renderTarget = nullptr;
			sf::Vector2u targetSize{};
			sf::Vector2f w00{};
			sf::Vector2f wx{};
			sf::Vector2f wy{};
			float targetHeight = 0.f;
		};

		CameraUniformState s_camUniforms{};
		bool s_haveCamUniforms = false;

		const sf::BlendMode& LightingBlendModeForPass() {
			if (SceneLighting::GetInstance().GetBlendMode() == LightingBlendMode::Screen) {
				static const sf::BlendMode screenBlend(sf::BlendMode::Factor::One,
				    sf::BlendMode::Factor::OneMinusSrcColor, sf::BlendMode::Equation::Add, sf::BlendMode::Factor::One,
				    sf::BlendMode::Factor::OneMinusSrcAlpha, sf::BlendMode::Equation::Add);
				return screenBlend;
			}
			return sf::BlendAdd;
		}

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

		struct ShapeFragUniforms
		{
			int shapeKind = 2;
			int vertCount = 0;
			std::array<sf::Vector2f, kMaxConvexVerts> convexVerts{};
			sf::Vector2f circleCenter{};
			float circleRadius = 1.f;
			sf::Vector2f rectMin{};
			sf::Vector2f rectMax{1.f, 1.f};
		};

		void FillShapeFragUniformsFromSfShape(const sf::Shape* shape, ShapeFragUniforms& out) {
			if (!shape) {
				return;
			}
			out.shapeKind = 2;
			out.vertCount = 0;
			if (const auto* convex = dynamic_cast<const sf::ConvexShape*>(shape)) {
				out.shapeKind = 0;
				const unsigned pc = convex->getPointCount();
				out.vertCount = static_cast<int>(std::min<std::size_t>(pc, kMaxConvexVerts));
				for (int i = 0; i < out.vertCount; ++i) {
					out.convexVerts[static_cast<std::size_t>(i)] = convex->getPoint(static_cast<unsigned>(i));
				}
			}
			else if (const auto* circle = dynamic_cast<const sf::CircleShape*>(shape)) {
				out.shapeKind = 1;
				out.circleCenter = circle->getGeometricCenter();
				out.circleRadius = circle->getRadius();
			}
			else if (const auto* rect = dynamic_cast<const sf::RectangleShape*>(shape)) {
				out.shapeKind = 2;
				const sf::Vector2f size = rect->getSize();
				out.rectMin = {0.f, 0.f};
				out.rectMax = {size.x, size.y};
			}
			else {
				const sf::FloatRect lb = shape->getLocalBounds();
				out.rectMin = lb.position;
				out.rectMax = {lb.position.x + lb.size.x, lb.position.y + lb.size.y};
			}
		}

	} // namespace

	void DrawShapeLightingPass(const ShapeVisualBase& visual, sf::RenderTarget& target, sf::RenderStates states) {
		if (!SceneLighting::GetInstance().IsEnabled()) {
			return;
		}

		const auto node = visual.GetNode();
		if (!node) {
			return;
		}
		const auto recv = node->FindBehaviour<ShapeLightReceiverBehaviour>();
		if (!recv || !recv->IsReceiverLightingEnabled()) {
			return;
		}
		sf::Shader* shader = GetShapeLightingShader();
		if (!shader) {
			return;
		}
		const sf::Shape* shape = visual.GetBaseShape();
		if (!shape) {
			return;
		}

		thread_local std::vector<GpuPointLight> tlsLights;
		tlsLights.clear();
		const std::size_t lightPool = SceneLighting::GetInstance().GetLights().size();
		if (tlsLights.capacity() < lightPool) {
			tlsLights.reserve(lightPool);
		}

		const sf::FloatRect worldBounds = node->GetWorldTransform().transformRect(visual.GetGlobalBounds());
		SceneLighting::GetInstance().SelectLightsForBounds(worldBounds, node.get(), tlsLights, kMaxLights);

		// Match SFML draw: states.transform * shape.getTransform() (see Utils::IsWorldPointInsideOfShape).
		const sf::Transform worldFromShapeLocal = states.transform * shape->getTransform();
		const sf::Glsl::Mat3 localFromWorld = worldFromShapeLocal.getInverse();

		const sf::Vector2u targetSize = target.getSize();
		const sf::Vector2f w00 = target.mapPixelToCoords({0, 0});
		const sf::Vector2f wx = target.mapPixelToCoords({1, 0});
		const sf::Vector2f wy = target.mapPixelToCoords({0, 1});

		std::array<sf::Glsl::Vec2, kMaxLights> lightPos{};
		std::array<sf::Glsl::Vec3, kMaxLights> lightCol{};
		std::array<float, kMaxLights> lightRad{};

		const std::size_t n = tlsLights.size();
		for (std::size_t i = 0; i < n; ++i) {
			lightPos[i] = sf::Glsl::Vec2(tlsLights[i].position.x, tlsLights[i].position.y);
			lightCol[i] = tlsLights[i].color;
			lightRad[i] = tlsLights[i].radius;
		}

		const sf::Color fill = visual.GetFillColor();
		const sf::Glsl::Vec4 fillV(static_cast<float>(fill.r) / 255.f, static_cast<float>(fill.g) / 255.f,
		    static_cast<float>(fill.b) / 255.f, static_cast<float>(fill.a) / 255.f);

		ShapeFragUniforms gpu{};
		FillShapeFragUniformsFromSfShape(shape, gpu);

		std::array<sf::Glsl::Vec2, kMaxConvexVerts> verts{};
		for (int i = 0; i < gpu.vertCount; ++i) {
			const sf::Vector2f& p = gpu.convexVerts[static_cast<std::size_t>(i)];
			verts[static_cast<std::size_t>(i)] = sf::Glsl::Vec2(p.x, p.y);
		}

		const sf::Glsl::Vec2 circleCenter(gpu.circleCenter.x, gpu.circleCenter.y);
		const sf::Glsl::Vec2 rectMin(gpu.rectMin.x, gpu.rectMin.y);
		const sf::Glsl::Vec2 rectMax(gpu.rectMax.x, gpu.rectMax.y);

		shader->setUniform(kUFillColor, fillV);

		const void* const targetKey = static_cast<const void*>(&target);
		const bool camSame = s_haveCamUniforms && s_camUniforms.renderTarget == targetKey &&
		                     s_camUniforms.targetSize == targetSize && s_camUniforms.w00 == w00 &&
		                     s_camUniforms.wx == wx && s_camUniforms.wy == wy &&
		                     s_camUniforms.targetHeight == static_cast<float>(targetSize.y);
		if (!camSame) {
			shader->setUniform(kUWorldOrigin, sf::Glsl::Vec2(w00.x, w00.y));
			shader->setUniform(kUWorldDx, sf::Glsl::Vec2(wx.x - w00.x, wx.y - w00.y));
			shader->setUniform(kUWorldDy, sf::Glsl::Vec2(wy.x - w00.x, wy.y - w00.y));
			shader->setUniform(kUTargetHeight, static_cast<float>(targetSize.y));
			s_camUniforms.renderTarget = targetKey;
			s_camUniforms.targetSize = targetSize;
			s_camUniforms.w00 = w00;
			s_camUniforms.wx = wx;
			s_camUniforms.wy = wy;
			s_camUniforms.targetHeight = static_cast<float>(targetSize.y);
			s_haveCamUniforms = true;
		}

		shader->setUniform(kULocalFromWorld, localFromWorld);
		shader->setUniform(kUShapeKind, gpu.shapeKind);
		shader->setUniform(kUVertexCount, gpu.vertCount);
		if (gpu.shapeKind == 0 && gpu.vertCount > 0) {
			shader->setUniformArray(kUVertices, verts.data(), static_cast<std::size_t>(gpu.vertCount));
		}

		shader->setUniform(kUCircleCenter, circleCenter);
		shader->setUniform(kUCircleRadius, gpu.circleRadius);
		shader->setUniform(kURectMin, rectMin);
		shader->setUniform(kURectMax, rectMax);

		shader->setUniform(kULightCount, static_cast<int>(n));
		if (n > 0) {
			shader->setUniformArray(kULightPos, lightPos.data(), n);
			shader->setUniformArray(kULightColor, lightCol.data(), n);
			shader->setUniformArray(kULightRadius, lightRad.data(), n);
		}

		shader->setUniform(kUModeBevel, recv->IsBevelEmbossMode() ? 1 : 0);
		shader->setUniform(kUBevelWidth, recv->GetBevelWidth());
		shader->setUniform(kUEaseCirc, recv->IsEaseInCirc() ? 1 : 0);
		shader->setUniform(kUDiffusion, recv->GetDiffusion());
		shader->setUniform(kULightingStrength, recv->GetLightingStrength());

		states.shader = shader;
		states.blendMode = LightingBlendModeForPass();
		target.draw(*shape, states);
	}

} // namespace Engine
