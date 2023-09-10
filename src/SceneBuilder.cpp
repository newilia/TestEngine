#include "SceneBuilder.h"

#include "BodyDebugComponent.h"
#include "FpsNode.h"
#include "AbstractShapeBody.h"
#include "EngineInterface.h"
#include "Scene.h"
#include "ShapeBody.h"
#include "UserInput.h"
#include "fmt/format.h"

std::shared_ptr<Scene> SceneBuilder::buildScene() {
	auto scene = make_shared<Scene>();
	sf::Vector2f screenSize(EI()->getMainWindow()->getSize());
	constexpr float wallActualWidth = 200;
	constexpr float wallVisibleWidth = 30;
	constexpr float wallOffset = wallActualWidth / 2 - wallVisibleWidth;
	float bodiesRestitution = 0.8f;

	std::string wallNames[] = { "bottom", "top", "left", "right" };
	sf::Vector2f wallSizes[] = {
		{ screenSize.x, wallActualWidth },
		{ screenSize.x, wallActualWidth },
		{ wallActualWidth, screenSize.y },
		{ wallActualWidth, screenSize.y }
	};
	sf::Vector2f wallPositions[] = {
		{ screenSize.x / 2, screenSize.y + wallOffset },
		{ screenSize.x / 2, -wallOffset },
		{ -wallOffset, screenSize.y / 2 },
		{ screenSize.x + wallOffset, screenSize.y / 2 }
	};
	for (int i = 0; i < 4; ++i) {
		auto wall = make_shared<RectangleBody>();
		wall->setName(wallNames[i]);
		wall->getShape()->setPosition(wallPositions[i]);
		wall->getShape()->setSize(wallSizes[i]);
		wall->getShape()->setFillColor(sf::Color(30, 255, 30, 50));
		wall->getShape()->setOutlineColor(sf::Color(30, 255, 30, 120));
		wall->getShape()->setOutlineThickness(1.f);
		wall->getPhysicalComponent()->setImmovable();
		wall->getPhysicalComponent()->mRestitution = bodiesRestitution;
		wall->init();
		scene->addChild(wall);
	}

	for (int i = 0; i < 30; ++i) {
		auto body = make_shared<CircleBody>();
		body->setName(fmt::format("circle_{}", i));

		float radius = 10 + rand() % 40;
		body->getShape()->setRadius(radius);
		constexpr float pointsCountConstant = 1.5f;
		auto pointsCount = pointsCountConstant * (7 + radius / 8);
		body->getShape()->setPointCount(pointsCount);

		constexpr sf::Uint8 minBr = 100;
		constexpr sf::Uint8 remainderBr = 255 - minBr;
		sf::Color color(minBr + rand() % remainderBr, minBr + rand() % remainderBr, minBr + rand() % remainderBr, 150);
		auto outlineColor = color;
		outlineColor.a = 255;

		body->getShape()->setFillColor(color);
		body->getShape()->setOutlineColor(outlineColor);
		body->getShape()->setOutlineThickness(1.f);
		body->getShape()->setPosition(rand() % static_cast<int>(screenSize.x ),rand() % static_cast<int>(screenSize.y));
		body->getPhysicalComponent()->mMass = 3.14f * radius * radius;
		body->getPhysicalComponent()->mRestitution = bodiesRestitution;
		body->requireComponent<BodyDebugComponent>();
		body->init();
		scene->addChild(body);
	}
	scene->addChild(make_shared<FpsNode>());
	return scene;
}
