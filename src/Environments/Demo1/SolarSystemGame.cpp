#include "SolarSystemGame.h"

#include "Engine/Behaviour/ButtonBehaviour.h"
#include "Engine/Core/FontManager.h"
#include "Engine/Core/MainContext.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Transform.h"
#include "Engine/Core/Utils.h"
#include "Engine/Visual/RectangleShapeVisual.h"
#include "Engine/Visual/TextVisual.h"
#include "SolarSystemBehaviour.h"

#include <SFML/Graphics/RectangleShape.hpp>

#include <memory>

using std::make_shared;
using std::shared_ptr;

namespace Demo1 {

	shared_ptr<SceneNode> CreateSolarSystemGameNode() {
		auto* window = Engine::MainContext::GetInstance().GetMainWindow();
		if (!window) {
			return nullptr;
		}

		auto mainRoot = make_shared<SceneNode>();
		mainRoot->SetName("SolarSystem");

		auto solarRoot = make_shared<SceneNode>();
		solarRoot->SetName("SolarSystemRoot");
		mainRoot->AddChild(solarRoot);

		auto control = make_shared<SceneNode>();
		control->SetName("SolarControl");
		auto rectVisual = make_shared<RectangleShapeVisual>();
		control->SetVisual(rectVisual);
		auto* rect = dynamic_cast<sf::RectangleShape*>(rectVisual->GetShape());
		if (rect) {
			rect->setSize({140.f, 48.f});
			rect->setOrigin(Utils::FindCenterOfMass(rect));
			rect->setFillColor(sf::Color(40, 90, 140, 220));
			rect->setOutlineColor(sf::Color(200, 220, 255, 255));
			rect->setOutlineThickness(2.f);
		}

		if (auto* font = Engine::MainContext::GetInstance().GetFontManager()->GetDefaultFont()) {
			auto labelVisual = make_shared<TextVisual>();
			labelVisual->Init(*font, "Restart", 22);
			labelVisual->SetFillColor(sf::Color(235, 240, 255, 255));
			const auto lb = labelVisual->GetLocalBounds();
			labelVisual->SetOrigin({lb.position.x + lb.size.x * 0.5f, lb.position.y + lb.size.y * 0.5f});
			auto labelNode = make_shared<SceneNode>();
			labelNode->SetName("SolarControlLabel");
			labelNode->SetVisual(std::move(labelVisual));
			control->AddChild(labelNode);
		}

		auto button = make_shared<ButtonBehaviour>();
		control->AddBehaviour(button);

		auto solarBeh = control->RequireBehaviour<SolarSystemBehaviour>();
		solarBeh->SetSolarSystemRoot(solarRoot);

		[[maybe_unused]] const auto subscription =
		    button->GetOnTapSignal().Subscribe([weakSolar = std::weak_ptr<SolarSystemBehaviour>(solarBeh)]() {
			    if (auto s = weakSolar.lock()) {
				    s->Restart();
			    }
		    });

		mainRoot->AddChild(control);
		control->GetLocalTransform()->SetPosition({0.f, -320.f});

		return mainRoot;
	}

} // namespace Demo1
