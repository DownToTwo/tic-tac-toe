// TODO: implement AI/minimax(difficult), insert AI in logic(impossible)
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
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
#include <string>
#include "ftxui-grid-container/grid-container.hpp"

using namespace ftxui;
constexpr uint8_t GRIDSIZE = 9;
struct Scores {
  int x = 0, o = 0;
};
using ButtonStateGrid = std::array<std::string, GRIDSIZE>;
enum STATE { X = 'X', O = 'O', NONE = ' ' };
std::string checkWinner(const std::array<std::string, GRIDSIZE>& buttonsState) {
  const std::array<std::array<int, 3>, 8> winningCombinations = {{
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
  // TODO: use stl std::find_if
  for (const auto& combination : winningCombinations) {
    if (buttonsState[combination[0]] != " " &&
        std::all_of(combination.begin(), combination.end(), [&](int i) {
          return buttonsState[i] == buttonsState[combination[0]];
        })) {
      return buttonsState[combination[0]];
    }
  }
  return "";
}
void togglePlayer(std::string& currentPlayer) {
  currentPlayer = (currentPlayer == "X") ? "O" : "X";
}

void updateScoresAndOutput(const std::string& winner,
                           Scores& scores,
                           std::string& output) {
  if (winner == "X") {
    scores.x++;
  } else if (winner == "O") {
    scores.o++;
  }
  output = winner.empty() ? " Draw OX " : winner + " wins!";
}

void buttonCallback(size_t index,
                    ButtonStateGrid& buttonsState,
                    std::string& currentPlayer,
                    int& count,
                    std::string& output,
                    Scores& scores) {
  if (buttonsState[index] != " ") {
    return;
  }
  buttonsState[index] = currentPlayer;
  std::string winner = checkWinner(buttonsState);
  if (!winner.empty() || count == GRIDSIZE - 1) {
    updateScoresAndOutput(winner, scores, output);
    buttonsState.fill(" ");
    count = 0;
  } else {
    ++count;
    output = "Current player: " + currentPlayer;
  }

  togglePlayer(currentPlayer);
}
int main() {
  auto screen = ScreenInteractive::TerminalOutput();
  std::string currentPlayer = "X";
  std::string output = "Current player: " + currentPlayer;
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
                          text(output) | border,

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
