#include "TicTacToeGame.h"

#include "Engine/Core/FontManager.h"
#include "Engine/Core/MainContext.h"
#include "Engine/Core/SceneNode.h"
#include "Engine/Core/Transform.h"
#include "Engine/Sorting/SortingStrategy.h"
#include "Engine/Visual/RectangleShapeVisual.h"
#include "Engine/Visual/TextVisual.h"
#include "TicTacToeBehaviour.h"

#include <SFML/Graphics/Text.hpp>

#include <fmt/format.h>

#include <array>
#include <cassert>

namespace Demo1 {

	namespace {
		constexpr float kCell = 96.f;
		constexpr float kLine = 8.f;
		constexpr float kBoard = 3.f * kCell + 2.f * kLine;

		/// Маркеры X/O не должны перехватывать тапы — хит только у ячейки.
		class TicTacToeMarkerTextVisual final : public TextVisual
		{
		public:
			using TextVisual::TextVisual;

			bool HitTest(const sf::Vector2f& /*worldPoint*/) const override {
				return false;
			}
		};

		class TicTacToeCellHitVisual final : public RectangleShapeVisual
		{
		public:
			void OnTap(const sf::Vector2f& /*worldPoint*/) override {
				if (auto b = _behaviour.lock()) {
					b->OnCellTapped(_cellIndex);
				}
			}

			void Configure(int cellIndex, std::weak_ptr<TicTacToeBehaviour> behaviour) {
				_cellIndex = cellIndex;
				_behaviour = std::move(behaviour);
				SetTapHandlingEnabled(true);
			}

		private:
			int _cellIndex = -1;
			std::weak_ptr<TicTacToeBehaviour> _behaviour;
		};

		void AddGridLine(const std::shared_ptr<SceneNode>& root, sf::Vector2f pos, sf::Vector2f size) {
			auto node = std::make_shared<SceneNode>();
			node->GetLocalTransform()->SetPosition(pos);
			auto rect = std::make_shared<RectangleShapeVisual>();
			rect->GetShape()->setSize(size);
			rect->GetShape()->setFillColor(sf::Color(220, 220, 220));
			node->SetVisual(std::move(rect));
			auto sorting = std::make_shared<RelativeSortingStrategy>();
			sorting->SetPriority(0);
			node->SetSortingStrategy(std::move(sorting));
			root->AddChild(node);
		}
	} // namespace

	std::shared_ptr<SceneNode> CreateTicTacToeGameNode() {
		auto root = std::make_shared<SceneNode>();
		root->SetName("TicTacToe");

		auto& ctx = Engine::MainContext::GetInstance();
		auto* font = ctx.GetFontManager()->GetDefaultFont();
		assert(font);

		auto hudText = std::make_shared<sf::Text>(*font, "", 18u);
		hudText->setFillColor(sf::Color::White);
		auto hudNode = std::make_shared<SceneNode>();
		hudNode->SetName("TicTacToeHUD");
		hudNode->GetLocalTransform()->SetPosition({0.f, -52.f});
		hudNode->SetVisual(std::make_shared<TextVisual>(hudText));
		root->AddChild(hudNode);

		AddGridLine(root, {kCell, 0.f}, {kLine, kBoard});
		AddGridLine(root, {2.f * kCell + kLine, 0.f}, {kLine, kBoard});
		AddGridLine(root, {0.f, kCell}, {kBoard, kLine});
		AddGridLine(root, {0.f, 2.f * kCell + kLine}, {kBoard, kLine});

		std::array<std::shared_ptr<sf::Text>, 9> cellTexts{};
		for (int i = 0; i < 9; ++i) {
			cellTexts[static_cast<std::size_t>(i)] = std::make_shared<sf::Text>(*font, "", 44u);
			cellTexts[static_cast<std::size_t>(i)]->setFillColor(sf::Color::White);
			cellTexts[static_cast<std::size_t>(i)]->setString("X");
			const sf::FloatRect b = cellTexts[static_cast<std::size_t>(i)]->getLocalBounds();
			cellTexts[static_cast<std::size_t>(i)]->setOrigin(
			    {b.position.x + b.size.x * 0.5f, b.position.y + b.size.y * 0.5f});
			cellTexts[static_cast<std::size_t>(i)]->setString("");
		}

		auto behaviour = std::make_shared<TicTacToeBehaviour>(hudText, cellTexts);
		root->AddBehaviour(behaviour);

		for (int i = 0; i < 9; ++i) {
			const int row = i / 3;
			const int col = i % 3;
			auto cellNode = std::make_shared<SceneNode>();
			cellNode->SetName(fmt::format("TicTacToeCell{}", i));
			const float x = static_cast<float>(col) * (kCell + kLine);
			const float y = static_cast<float>(row) * (kCell + kLine);
			cellNode->GetLocalTransform()->SetPosition({x, y});

			auto hit = std::make_shared<TicTacToeCellHitVisual>();
			hit->GetShape()->setSize({kCell, kCell});
			hit->GetShape()->setFillColor(sf::Color(0, 0, 0, 0));
			hit->Configure(i, behaviour);
			cellNode->SetVisual(std::move(hit));

			auto cellSort = std::make_shared<RelativeSortingStrategy>();
			cellSort->SetPriority(10);
			cellNode->SetSortingStrategy(std::move(cellSort));

			auto markerNode = std::make_shared<SceneNode>();
			markerNode->GetLocalTransform()->SetPosition({kCell * 0.5f, kCell * 0.5f});
			markerNode->SetVisual(std::make_shared<TicTacToeMarkerTextVisual>(cellTexts[static_cast<std::size_t>(i)]));
			auto markerSort = std::make_shared<RelativeSortingStrategy>();
			markerSort->SetPriority(20);
			markerNode->SetSortingStrategy(std::move(markerSort));
			cellNode->AddChild(markerNode);

			root->AddChild(cellNode);
		}

		return root;
	}

} // namespace Demo1
