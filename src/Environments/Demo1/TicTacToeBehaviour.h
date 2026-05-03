#pragma once

#include "Engine/Behaviour/Behaviour.h"
#include "Engine/Core/MetaClass.h"

#include <array>
#include <memory>
#include <random>

namespace sf {
	class Text;
}

enum class TicTacToeTurnPhase : std::uint8_t
{
	PlayerTurn,
	AiThinking,
	BetweenRounds,
};

/// Локальный контроллер партий «крестики-нолики» для Demo1 (игрок = X, ИИ = O).
class TicTacToeBehaviour : public Behaviour
{
	META_CLASS()

public:
	explicit TicTacToeBehaviour(std::shared_ptr<sf::Text> hudText, std::array<std::shared_ptr<sf::Text>, 9> cellTexts);

	void OnInit() override;
	void OnUpdate(const sf::Time& dt) override;

	void OnCellTapped(int cellIndex);

private:
	void RefreshHud();
	void BeginNewRound();
	void TryPlayerMove(int cellIndex);
	void AdvanceAiTurn();
	void ApplyMove(int cellIndex, std::uint8_t mark);
	void ClearCellVisuals();
	void UpdateCellVisual(int cellIndex);
	void EndRoundFromTerminal(bool playerWon, bool aiWon, bool draw);

	static std::uint8_t WinnerMark(const std::array<std::uint8_t, 9>& board);
	static bool BoardFull(const std::array<std::uint8_t, 9>& board);
	static int ChooseAiMoveMinimax(const std::array<std::uint8_t, 9>& board);
	static int MinimaxScore(std::array<std::uint8_t, 9>& board, bool aiTurn);

private:
	/// @property(name="AI mistake probability", minValue=0.f, maxValue=1.f)
	float _aiMistakeProbability = 0.2f;
	/// @property(name="AI think delay (s)", minValue=0.f, maxValue=5.f)
	float _aiThinkDelaySeconds = 2.f;

private:
	std::shared_ptr<sf::Text> _hudText;
	std::array<std::shared_ptr<sf::Text>, 9> _cellTexts;
	std::array<std::uint8_t, 9> _board{};
	TicTacToeTurnPhase _phase = TicTacToeTurnPhase::PlayerTurn;
	float _aiThinkElapsed = 0.f;
	float _betweenRoundsElapsed = 0.f;
	int _playerWins = 0;
	int _aiWins = 0;
	int _draws = 0;
	bool _playerStartsNextRound = true;
	bool _playerStartedThisRound = true;
	std::mt19937 _rng{std::random_device{}()};
};
