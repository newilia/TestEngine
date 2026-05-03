#pragma once

#include "Engine/Editor/Tools/IEditorTool.h"

class TapTool final : public IEditorTool
{
public:
	[[nodiscard]] bool processEvent(const sf::Event& event) override;
	void drawToolParametersUi() override;
};
