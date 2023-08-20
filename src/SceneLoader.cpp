#include "SceneLoader.h"
#include "PolygonBody.h"
#include "Scene.h"

SceneLoader::SceneLoader(Scene* scene)
	: mScene(scene)
{
}

void SceneLoader::loadScene() {
	auto rect = make_shared<RectangleBody>();
	rect->setId("Rect");
	rect->init();
	sf::Vector2f size1(100, 100);
	rect->getShape()->setSize(size1);
	rect->getShape()->setFillColor(sf::Color::Red);
	rect->getPhysicalComponent()->mMass = size1.x * size1.y;
	mScene->addChild(rect);


	auto rect2 = make_shared<RectangleBody>();
	rect2->setId("Rect2");
	rect2->init();
	sf::Vector2f size2(100, 100);
	rect2->getShape()->setSize(size2);
	rect2->getShape()->setFillColor(sf::Color::Blue);
	rect2->getPhysicalComponent()->mPos = { 0, 150 };
	rect2->getPhysicalComponent()->mMass = size2.x * size2.y;
	mScene->addChild(rect2);

}
