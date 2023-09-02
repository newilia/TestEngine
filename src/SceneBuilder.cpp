#include "SceneBuilder.h"

#include "DebugComponent.h"
#include "FpsNode.h"
#include "PolygonBody.h"
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
		rect->getShape()->setFillColor(sf::Color::Red);
		rect->getPhysicalComponent()->mMass = size.x * size.y;
		rect->requireComponent<DebugComponent>();
		scene->addChild(rect);
	}
	
	{
		auto rect = make_shared<RectangleBody>();
		rect->setId("Rect2");
		rect->init();
		sf::Vector2f size2(100, 100);
		rect->getShape()->setSize(size2);
		rect->getShape()->setFillColor(sf::Color::Blue);
		rect->getPhysicalComponent()->mPos = { 50, 50 };
		rect->getPhysicalComponent()->mMass = size2.x * size2.y;
		rect->requireComponent<DebugComponent>();
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
