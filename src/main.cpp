// TODO: implement AI/minimax(difficult)
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/deprecated.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/node.hpp>
#include <ftxui/screen/color.hpp>
#include <ftxui/screen/screen.hpp>
#include <functional>
#include <iostream>
#include <random>
#include <string>
#include <string_view>
#include <vector>
#include "ftxui-grid-container/grid-container.hpp"

using namespace ftxui;
constexpr auto GRIDSIZE = 9;
struct Scores {
  uint64_t x = 0, o = 0;
};
enum class StateEnum { X = 0, O, NONE, COUNT };

constexpr std::array<std::string_view, 3> State = {"X", "O", " "};
constexpr std::array<std::string_view, 3> WinnerMessage = {"X wins!", "O wins!",
                                                           "Draw OX"};
constexpr std::array<std::string_view, 2> CurrentPlayer = {"Current player: X",
                                                           "Current player: O"};
constexpr size_t idx(StateEnum state) {
  return static_cast<size_t>(state);
}

using ButtonStateGrid = std::array<std::string, GRIDSIZE>;
StateEnum checkWinner(const ButtonStateGrid& gameState) {
  constexpr std::array<std::array<int, 3>, 8> winningCombinations = {{
      // 8 magic number for all possible solutions for 3x3 table
      std::array<int, 3>{0, 1, 2},  // Row 1
      std::array<int, 3>{3, 4, 5},  // Row 2
      std::array<int, 3>{6, 7, 8},  // Row 3
      std::array<int, 3>{0, 3, 6},  // Column 1
      std::array<int, 3>{1, 4, 7},  // Column 2
      std::array<int, 3>{2, 5, 8},  // Column 3
      std::array<int, 3>{0, 4, 8},  // Diagonal 1
      std::array<int, 3>{2, 4, 6}   // Diagonal 2
  }};
  for (const auto& combination : winningCombinations) {
    if (gameState[combination[0]] != State[idx(StateEnum::NONE)] &&
        std::all_of(combination.begin(), combination.end(), [&](size_t i) {
          return gameState[i] == gameState[combination[0]];
        })) {
      return gameState[combination[0]] == State[idx(StateEnum::X)]
                 ? StateEnum::X
                 : StateEnum::O;
    }
  }
  return StateEnum::NONE;
}
void togglePlayer(StateEnum& currentPlayer) {
  currentPlayer = (currentPlayer == StateEnum::X) ? StateEnum::O : StateEnum::X;
}

void updateScoresAndOutput(const StateEnum& winner,
                           Scores& scores,
                           std::string_view& output) {
  if (winner == StateEnum::X) {
    scores.x++;
  } else if (winner == StateEnum::O) {
    scores.o++;
  }
  output = WinnerMessage[idx(winner)];
}

int bot(const ButtonStateGrid& gameState, StateEnum player) {
  // Check if AI can win in the next move.
  for (size_t i = 0; i < GRIDSIZE; ++i) {
    if (gameState[i] == State[idx(StateEnum::NONE)]) {
      ButtonStateGrid tempState = gameState;
      tempState[i] = State[idx(player)];
      if (checkWinner(tempState) == player) {
        return i;
      }
    }
  }

  // Check if opponent can win in the next move and block.
  StateEnum opponent = (player == StateEnum::X) ? StateEnum::O : StateEnum::X;
  for (size_t i = 0; i < GRIDSIZE; ++i) {
    if (gameState[i] == State[idx(StateEnum::NONE)]) {
      ButtonStateGrid tempState = gameState;
      tempState[i] = State[idx(opponent)];
      if (checkWinner(tempState) == opponent) {
        return i;
      }
    }
  }

  // Choose a random available position.
  std::vector<size_t> availableMoves;
  for (size_t i = 0; i < GRIDSIZE; ++i) {
    if (gameState[i] == State[idx(StateEnum::NONE)]) {
      availableMoves.push_back(i);
    }
  }
  if (!availableMoves.empty()) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, availableMoves.size() - 1);
    return availableMoves[dis(gen)];
  }

  // No available moves (unlikely to happen in normal gameplay).
  return -1;
}
size_t findBestMove(const ButtonStateGrid& gameState) {
  return bot(gameState, StateEnum::O);  // AI player always plays 'O'
}
void buttonCallback(size_t index,
                    ButtonStateGrid& gameState,
                    StateEnum& currentPlayer,
                    int& count,
                    std::string_view& output,
                    Scores& scores) {
  if (gameState[index] != State[idx(StateEnum::NONE)]) {
    return;
  }
  gameState[index] = State[idx(currentPlayer)];
  StateEnum winner = checkWinner(gameState);
  if (winner != StateEnum::NONE || count == GRIDSIZE - 1) {
    updateScoresAndOutput(winner, scores, output);
    gameState.fill(std::string(State[idx(StateEnum::NONE)]));
    count = 0;
  } else {
    if (currentPlayer == StateEnum::X) {
      ++count;
      output = CurrentPlayer[idx(currentPlayer)];
      // After player's move, AI should make a move
      size_t aiMove = findBestMove(gameState);
      gameState[aiMove] =
          State[idx(StateEnum::O)];  // AI player always plays 'O'
      winner = checkWinner(gameState);
      if (winner != StateEnum::NONE || count == GRIDSIZE - 1) {
        updateScoresAndOutput(winner, scores, output);
        gameState.fill(std::string(State[idx(StateEnum::NONE)]));
        count = 0;
      } else {
        ++count;
        output = CurrentPlayer[idx(currentPlayer)];
      }
    } else {
      // Current player is AI, just update output and toggle player
      togglePlayer(currentPlayer);
      ++count;
      output = CurrentPlayer[idx(currentPlayer)];
    }
  }
}
int main() {
  auto screen = ScreenInteractive::TerminalOutput();
  StateEnum currentPlayer = StateEnum::X;
  std::string_view output = CurrentPlayer[idx(currentPlayer)];
  auto style = size(WIDTH, EQUAL, 8);
  Components buttons(GRIDSIZE);
  ButtonStateGrid gameState;
  Scores scores;
  gameState.fill(" ");
  int count = 0;
  for (size_t i = 0; i < GRIDSIZE; ++i) {
    buttons[i] = Button(&gameState[i],
                        std::bind(buttonCallback, i, std::ref(gameState),
                                  std::ref(currentPlayer), std::ref(count),
                                  std::ref(output), std::ref(scores))) |
                 style;
  }
  // Maybe i can improve it
  auto grid = GridContainer({{buttons.begin(), buttons.begin() + 3},
                             {buttons.begin() + 3, buttons.begin() + 6},
                             {buttons.begin() + 6, buttons.end()}});

  auto scoresElement = [&]() -> Element {
    return hbox({text("X:"), text(std::to_string(scores.x)), text("|"),
                 text("O:"), text(std::to_string(scores.o))});
  };
  auto renderer = Renderer(grid, [&] {
    return window(text("TicTacToe") | center,
                  {
                      vbox({
                          text(std::string(output)) | border,

                          scoresElement() | center,
                          grid->Render() | center | flex,
                      }),
                  }) |
           size(WIDTH, EQUAL, 30) | center;
  });
  screen.Loop(renderer);
  std::cout << "HEY";
  return 0;
}
