#include "SceneBuilder.h"

#include "BodyDebugComponent.h"
#include "FpsNode.h"
#include "AbstractShapeBody.h"
#include "Scene.h"
#include "UserInput.h"


std::shared_ptr<Scene> SceneBuilder::buildScene() {
	auto scene = make_shared<Scene>();
	
	{
		auto body = make_shared<RectangleBody>();
		body->setId("Rect");
		body->init();
		sf::Vector2f size(100, 100);
		body->getShape()->setSize(size);
		body->getShape()->setFillColor(sf::Color(255, 30, 30, 50));
		body->getShape()->setOutlineColor(sf::Color(255, 30, 30, 120));
		body->getShape()->setOutlineThickness(1.f);
		body->getPhysicalComponent()->mPos = { 200, 200 };
		body->getPhysicalComponent()->mMass = size.x * size.y;
		body->getPhysicalComponent()->mRestitution = 0.5f;
		body->requireComponent<BodyDebugComponent>();
		scene->addChild(body);
	}
	
	{
		auto body = make_shared<RectangleBody>();
		body->setId("Rect2");
		body->init();
		sf::Vector2f size2(100, 100);
		body->getShape()->setSize(size2);
		body->getShape()->setFillColor(sf::Color(30, 255, 30, 50));
		body->getShape()->setOutlineColor(sf::Color(30, 255, 30, 120));
		body->getShape()->setOutlineThickness(1.f);
		body->getPhysicalComponent()->mPos = { 350, 200 };
		body->getPhysicalComponent()->mMass = size2.x * size2.y;
		body->requireComponent<BodyDebugComponent>();
		scene->addChild(body);
	}

	{	// screen borders
		{
			auto body = make_shared<RectangleBody>();
			body->setId("bottom");
			body->init();
			sf::Vector2f size2(800, 100);
			body->getShape()->setSize(size2);
			body->getShape()->setFillColor(sf::Color(30, 255, 30, 50));
			body->getShape()->setOutlineColor(sf::Color(30, 255, 30, 120));
			body->getShape()->setOutlineThickness(1.f);
			body->getPhysicalComponent()->mPos = { 0, 550 };
			body->getPhysicalComponent()->mMass = std::numeric_limits<float>::infinity();
			scene->addChild(body);
		}
		{
			auto body = make_shared<RectangleBody>();
			body->setId("top");
			body->init();
			sf::Vector2f size(800, 100);
			body->getShape()->setSize(size);
			body->getShape()->setFillColor(sf::Color(30, 255, 30, 50));
			body->getShape()->setOutlineColor(sf::Color(30, 255, 30, 120));
			body->getShape()->setOutlineThickness(1.f);
			body->getPhysicalComponent()->mPos = { 00, -50 };
			body->getPhysicalComponent()->mMass = std::numeric_limits<float>::infinity();
			scene->addChild(body);
		}
		{
			auto body = make_shared<RectangleBody>();
			body->setId("left");
			body->init();
			sf::Vector2f size2(100, 600);
			body->getShape()->setSize(size2);
			body->getShape()->setFillColor(sf::Color(30, 255, 30, 50));
			body->getShape()->setOutlineColor(sf::Color(30, 255, 30, 120));
			body->getShape()->setOutlineThickness(1.f);
			body->getPhysicalComponent()->mPos = { -50, 0 };
			body->getPhysicalComponent()->mMass = std::numeric_limits<float>::infinity();
			scene->addChild(body);
		}
		{
			auto body = make_shared<RectangleBody>();
			body->setId("right");
			body->init();
			sf::Vector2f size2(100, 600);
			body->getShape()->setSize(size2);
			body->getShape()->setFillColor(sf::Color(30, 255, 30, 50));
			body->getShape()->setOutlineColor(sf::Color(30, 255, 30, 120));
			body->getShape()->setOutlineThickness(1.f);
			body->getPhysicalComponent()->mPos = { 750, 0 };
			body->getPhysicalComponent()->mMass = std::numeric_limits<float>::infinity();
			scene->addChild(body);
		}
	}

	{
		auto body = make_shared<CircleBody>();
		body->setId("Circle1");
		body->init();
		float radius = 30.f;
		body->getShape()->setRadius(radius);
		body->getShape()->setFillColor(sf::Color(200, 100, 40, 50));
		body->getShape()->setOutlineColor(sf::Color(200, 100, 40, 120));
		body->getShape()->setOutlineThickness(1.f);
		body->getPhysicalComponent()->mPos = { 300, 300 };
		body->getPhysicalComponent()->mMass = 3.14 * radius * radius;
		scene->addChild(body);
	}

	{
		auto body = make_shared<CircleBody>();
		body->setId("Circle2");
		body->init();
		float radius = 70.f;
		body->getShape()->setRadius(radius);
		body->getShape()->setFillColor(sf::Color(100, 50, 255, 50));
		body->getShape()->setOutlineColor(sf::Color(100, 50, 255, 120));
		body->getShape()->setOutlineThickness(1.f);
		body->getPhysicalComponent()->mPos = { 500, 400 };
		body->getPhysicalComponent()->mMass = 3.14 * radius * radius;
		scene->addChild(body);
	}
	scene->addChild(make_shared<FpsNode>());
	return scene;
}
