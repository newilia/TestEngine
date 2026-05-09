#include "Engine/Editor/GameBackgroundWidget.h"

#include "Engine/Background/ParallaxTextureGameBackground.h"
#include "Engine/Background/PlainColorGameBackground.h"
#include "Engine/Core/IPropertiesProvider.h"
#include "Engine/Core/MainContext.h"
#include "Engine/Core/PropertyTree.h"

#include <SFML/Graphics/Color.hpp>

#include <imgui.h>

namespace Engine {

	void GameBackgroundWidget::Draw() const {
		auto ctx = MainContext::GetInstance().GetGameBackgroundContext();
		if (!ctx) {
			return;
		}

		const char* items[] = {"None", "Plain color", "Parallax texture"};
		int currentItem = 0;
		if (auto* bg = ctx->GetBackground()) {
			if (dynamic_cast<PlainColorGameBackground*>(bg) != nullptr) {
				currentItem = 1;
			}
			else if (dynamic_cast<ParallaxTextureGameBackground*>(bg) != nullptr) {
				currentItem = 2;
			}
		}

		int selected = currentItem;
		if (ImGui::Combo("Type", &selected, items, IM_ARRAYSIZE(items))) {
			if (selected == 0) {
				ctx->ClearBackground();
			}
			else if (selected == 1 && currentItem != 1) {
				ctx->SetPlainColorBackground(sf::Color(32, 32, 48));
			}
			else if (selected == 2 && currentItem != 2) {
				ctx->SetParallaxTextureBackground({}, 1.f, 0.f, 0.35f, 256.f);
			}
		}

		auto* bg = ctx->GetBackground();
		if (!bg) {
			return;
		}

		ImGui::SeparatorText("Properties");

		PropertyTree tree;
		PropertyBuilder builder(tree);
		static_cast<IPropertiesProvider*>(bg)->BuildPropertyTree(builder);
		if (tree.roots.empty() && tree.inspectorMethods.empty()) {
			return;
		}

		_propertyDrawer.Draw(tree, {.unwrapSingleRootObject = true});
	}

} // namespace Engine
