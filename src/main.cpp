// TODO: implement AI/minimax(difficult), insert AI in logic(impossible)
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
#include <string>
#include <string_view>
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
constexpr std::array<std::string_view, 3> CurrentPlayer = {"Current player: X",
                                                           "Current player: O"};
constexpr size_t idx(StateEnum state) {
  return static_cast<size_t>(state);
}

using ButtonStateGrid = std::array<std::string, GRIDSIZE>;

StateEnum checkWinner(const ButtonStateGrid& buttonsState) {
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
    if (buttonsState[combination[0]] != State[idx(StateEnum::NONE)] &&
        std::all_of(combination.begin(), combination.end(), [&](size_t i) {
          return buttonsState[i] == buttonsState[combination[0]];
        })) {
      return buttonsState[combination[0]] == State[idx(StateEnum::X)]
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

void buttonCallback(size_t index,
                    ButtonStateGrid& buttonsState,
                    StateEnum& currentPlayer,
                    int& count,
                    std::string_view& output,
                    Scores& scores) {
  if (buttonsState[index] != State[idx(StateEnum::NONE)]) {
    return;
  }
  buttonsState[index] = State[idx(currentPlayer)];
  StateEnum winner = checkWinner(buttonsState);
  if (winner != StateEnum::NONE || count == GRIDSIZE - 1) {
    updateScoresAndOutput(winner, scores, output);
    buttonsState.fill(std::string(State[idx(StateEnum::NONE)]));
    count = 0;
    togglePlayer(currentPlayer);

  } else {
    togglePlayer(currentPlayer);
    ++count;
    output = CurrentPlayer[idx(currentPlayer)];
  }
}
int main() {
  auto screen = ScreenInteractive::TerminalOutput();
  StateEnum currentPlayer = StateEnum::X;
  std::string_view output = CurrentPlayer[idx(currentPlayer)];
  auto style = size(WIDTH, EQUAL, 8);
  Components buttons(GRIDSIZE);
  ButtonStateGrid buttonsState;
  Scores scores;
  buttonsState.fill(" ");
  int count = 0;
  for (size_t i = 0; i < GRIDSIZE; ++i) {
    buttons[i] = Button(&buttonsState[i],
                        std::bind(buttonCallback, i, std::ref(buttonsState),
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
