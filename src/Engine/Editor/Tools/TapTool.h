#pragma once

#include "Engine/Editor/Tools/IEditorTool.h"

class TapTool final : public IEditorTool
{
public:
	[[nodiscard]] bool ProcessEvent(const sf::Event& event) override;
	void DrawToolParametersUi() override;
};
