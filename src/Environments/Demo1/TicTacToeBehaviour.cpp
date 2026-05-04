#include "TicTacToeBehaviour.h"

#include "TicTacToeBehaviour.generated.hpp"

#include <fmt/format.h>

#include <algorithm>
#include <cassert>
#include <vector>

namespace {

	constexpr float kBetweenRoundsDelay = 1.25f;
	constexpr std::uint8_t kEmpty = 0;
	constexpr std::uint8_t kX = 1;
	constexpr std::uint8_t kO = 2;

	constexpr int kLines[8][3] = {
	    {0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {0, 3, 6}, {1, 4, 7}, {2, 5, 8}, {0, 4, 8}, {2, 4, 6},
	};

} // namespace

TicTacToeBehaviour::TicTacToeBehaviour(std::shared_ptr<TextVisual> hudText,
                                       std::array<std::shared_ptr<TextVisual>, 9> cellTexts)
    : _hudText(std::move(hudText)), _cellTexts(std::move(cellTexts)) {}

void TicTacToeBehaviour::OnInit() {
	BeginNewRound();
}

void TicTacToeBehaviour::OnUpdate(const sf::Time& dt) {
	const float sec = dt.asSeconds();

	if (_phase == TicTacToeTurnPhase::AiThinking) {
		_aiThinkElapsed += sec;
		if (_aiThinkElapsed >= _aiThinkDelaySeconds) {
			_aiThinkElapsed = 0.f;
			AdvanceAiTurn();
		}
	}
	else if (_phase == TicTacToeTurnPhase::BetweenRounds) {
		_betweenRoundsElapsed += sec;
		if (_betweenRoundsElapsed >= kBetweenRoundsDelay) {
			_betweenRoundsElapsed = 0.f;
			BeginNewRound();
		}
	}

	RefreshHud();
}

void TicTacToeBehaviour::OnCellTapped(const int cellIndex) {
	if (cellIndex < 0 || cellIndex >= 9) {
		return;
	}
	TryPlayerMove(cellIndex);
}

void TicTacToeBehaviour::RefreshHud() {
	if (!_hudText) {
		return;
	}

	std::string status;
	switch (_phase) {
	case TicTacToeTurnPhase::PlayerTurn:
		status = "Your turn, brother!";
		break;
	case TicTacToeTurnPhase::AiThinking:
		status = "AI is thinking...";
		break;
	case TicTacToeTurnPhase::BetweenRounds:
		status = "Next round...";
		break;
	}

	_hudText->SetString(fmt::format("You: {}  AI: {}  Draws: {}\n{}", _playerWins, _aiWins, _draws, status));
}

void TicTacToeBehaviour::BeginNewRound() {
	_board.fill(kEmpty);
	ClearCellVisuals();
	_playerStartedThisRound = _playerStartsNextRound;
	if (_playerStartsNextRound) {
		_phase = TicTacToeTurnPhase::PlayerTurn;
	}
	else {
		_phase = TicTacToeTurnPhase::AiThinking;
		_aiThinkElapsed = 0.f;
	}
}

void TicTacToeBehaviour::TryPlayerMove(const int cellIndex) {
	if (_phase != TicTacToeTurnPhase::PlayerTurn) {
		return;
	}
	if (_board[static_cast<std::size_t>(cellIndex)] != kEmpty) {
		return;
	}

	ApplyMove(cellIndex, kX);

	const std::uint8_t w = WinnerMark(_board);
	if (w == kX) {
		EndRoundFromTerminal(true, false, false);
		return;
	}
	if (BoardFull(_board)) {
		EndRoundFromTerminal(false, false, true);
		return;
	}

	_phase = TicTacToeTurnPhase::AiThinking;
	_aiThinkElapsed = 0.f;
}

void TicTacToeBehaviour::AdvanceAiTurn() {
	assert(_phase == TicTacToeTurnPhase::AiThinking);

	std::vector<int> legal;
	for (int i = 0; i < 9; ++i) {
		if (_board[static_cast<std::size_t>(i)] == kEmpty) {
			legal.push_back(i);
		}
	}
	if (legal.empty()) {
		return;
	}

	std::uniform_real_distribution<float> dist01(0.f, 1.f);
	int chosen = ChooseAiMoveMinimax(_board);
	if (chosen < 0) {
		chosen = legal[0];
	}

	if (_aiMistakeProbability > 0.f && dist01(_rng) < _aiMistakeProbability) {
		std::uniform_int_distribution<std::size_t> pick(0, legal.size() - 1);
		chosen = legal[pick(_rng)];
	}

	ApplyMove(chosen, kO);

	const std::uint8_t w = WinnerMark(_board);
	if (w == kO) {
		EndRoundFromTerminal(false, true, false);
		return;
	}
	if (BoardFull(_board)) {
		EndRoundFromTerminal(false, false, true);
		return;
	}

	_phase = TicTacToeTurnPhase::PlayerTurn;
}

void TicTacToeBehaviour::ApplyMove(const int cellIndex, const std::uint8_t mark) {
	assert(cellIndex >= 0 && cellIndex < 9);
	assert(_board[static_cast<std::size_t>(cellIndex)] == kEmpty);
	_board[static_cast<std::size_t>(cellIndex)] = mark;
	UpdateCellVisual(cellIndex);
}

void TicTacToeBehaviour::ClearCellVisuals() {
	for (int i = 0; i < 9; ++i) {
		if (_cellTexts[static_cast<std::size_t>(i)]) {
			_cellTexts[static_cast<std::size_t>(i)]->SetString("");
		}
	}
}

void TicTacToeBehaviour::UpdateCellVisual(const int cellIndex) {
	auto& t = _cellTexts[static_cast<std::size_t>(cellIndex)];
	if (!t) {
		return;
	}
	const std::uint8_t v = _board[static_cast<std::size_t>(cellIndex)];
	if (v == kX) {
		t->SetString("X");
	}
	else if (v == kO) {
		t->SetString("O");
	}
	else {
		t->SetString("");
	}
}

void TicTacToeBehaviour::EndRoundFromTerminal(const bool playerWon, const bool aiWon, const bool draw) {
	if (playerWon) {
		++_playerWins;
		_playerStartsNextRound = true;
	}
	else if (aiWon) {
		++_aiWins;
		_playerStartsNextRound = false;
	}
	else if (draw) {
		++_draws;
		_playerStartsNextRound = !_playerStartedThisRound;
	}

	_phase = TicTacToeTurnPhase::BetweenRounds;
	_betweenRoundsElapsed = 0.f;
}

std::uint8_t TicTacToeBehaviour::WinnerMark(const std::array<std::uint8_t, 9>& board) {
	for (const auto& line : kLines) {
		const std::uint8_t a = board[static_cast<std::size_t>(line[0])];
		if (a == kEmpty) {
			continue;
		}
		if (a == board[static_cast<std::size_t>(line[1])] && a == board[static_cast<std::size_t>(line[2])]) {
			return a;
		}
	}
	return kEmpty;
}

bool TicTacToeBehaviour::BoardFull(const std::array<std::uint8_t, 9>& board) {
	return std::find(board.begin(), board.end(), kEmpty) == board.end();
}

int TicTacToeBehaviour::MinimaxScore(std::array<std::uint8_t, 9>& board, const bool aiTurn) {
	const std::uint8_t w = WinnerMark(board);
	if (w == kO) {
		return 10;
	}
	if (w == kX) {
		return -10;
	}
	if (BoardFull(board)) {
		return 0;
	}

	if (aiTurn) {
		int best = -100;
		for (int i = 0; i < 9; ++i) {
			if (board[static_cast<std::size_t>(i)] != kEmpty) {
				continue;
			}
			board[static_cast<std::size_t>(i)] = kO;
			best = std::max(best, MinimaxScore(board, false));
			board[static_cast<std::size_t>(i)] = kEmpty;
		}
		return best;
	}

	int best = 100;
	for (int i = 0; i < 9; ++i) {
		if (board[static_cast<std::size_t>(i)] != kEmpty) {
			continue;
		}
		board[static_cast<std::size_t>(i)] = kX;
		best = std::min(best, MinimaxScore(board, true));
		board[static_cast<std::size_t>(i)] = kEmpty;
	}
	return best;
}

int TicTacToeBehaviour::ChooseAiMoveMinimax(const std::array<std::uint8_t, 9>& board) {
	int bestMove = -1;
	int bestScore = -1000;
	for (int i = 0; i < 9; ++i) {
		if (board[static_cast<std::size_t>(i)] != kEmpty) {
			continue;
		}
		auto b = board;
		b[static_cast<std::size_t>(i)] = kO;
		const int score = MinimaxScore(b, false);
		if (score > bestScore) {
			bestScore = score;
			bestMove = i;
		}
	}
	return bestMove;
}
