#include "SceneBuilder.h"

#include "BodyDebugComponent.h"
#include "FpsNode.h"
#include "AbstractShapeBody.h"
#include "Scene.h"
#include "UserInput.h"


std::shared_ptr<Scene> SceneBuilder::buildScene() {
	auto scene = make_shared<Scene>();
	
	{
		auto rect = make_shared<RectangleBody>();
		rect->setId("Rect");
		rect->init();
		sf::Vector2f size(100, 100);
		rect->getShape()->setSize(size);
		rect->getShape()->setFillColor(sf::Color(255, 30, 30, 50));
		rect->getShape()->setOutlineColor(sf::Color(255, 30, 30, 120));
		rect->getShape()->setOutlineThickness(1.f);
		rect->getPhysicalComponent()->mPos = { 200, 200 };
		rect->getPhysicalComponent()->mMass = size.x * size.y;
		rect->getPhysicalComponent()->mRestitution = 0.5f;
		rect->requireComponent<BodyDebugComponent>();
		scene->addChild(rect);
	}
	
	{
		auto rect = make_shared<RectangleBody>();
		rect->setId("Rect2");
		rect->init();
		sf::Vector2f size2(100, 100);
		rect->getShape()->setSize(size2);
		rect->getShape()->setFillColor(sf::Color(30, 255, 30, 50));
		rect->getShape()->setOutlineColor(sf::Color(30, 255, 30, 120));
		rect->getShape()->setOutlineThickness(1.f);
		rect->getPhysicalComponent()->mPos = { 350, 200 };
		rect->getPhysicalComponent()->mMass = size2.x * size2.y;
		rect->requireComponent<BodyDebugComponent>();
		scene->addChild(rect);
	}

	{
		auto rect = make_shared<RectangleBody>();
		rect->setId("Rect2");
		rect->init();
		sf::Vector2f size2(800, 100);
		rect->getShape()->setSize(size2);
		rect->getShape()->setFillColor(sf::Color(30, 255, 30, 50));
		rect->getShape()->setOutlineColor(sf::Color(30, 255, 30, 120));
		rect->getShape()->setOutlineThickness(1.f);
		rect->getPhysicalComponent()->mPos = { 0, 500 };
		rect->getPhysicalComponent()->mMass = 9999999999.f;
		rect->requireComponent<BodyDebugComponent>();
		scene->addChild(rect);
	}

	/*{
		auto circle = make_shared<CircleBody>();
		circle->setId("Circle1");
		circle->init();
		float radius = 40.f;
		circle->getShape()->setRadius(radius);
		circle->getShape()->setFillColor(sf::Color(200, 100, 40));
		circle->getPhysicalComponent()->mPos = { 300, 300 };
		circle->getPhysicalComponent()->mMass = 3.14 * radius * radius;
		mScene->addChild(circle);
	}

	{
		auto circle = make_shared<CircleBody>();
		circle->setId("Circle2");
		circle->init();
		float radius = 120.f;
		circle->getShape()->setRadius(radius);
		circle->getShape()->setFillColor(sf::Color(100, 50, 200));
		circle->getPhysicalComponent()->mPos = { 500, 400 };
		circle->getPhysicalComponent()->mMass = 3.14 * radius * radius;
		mScene->addChild(circle);
	}*/
	scene->addChild(make_shared<FpsNode>());
	return scene;
}
