// TODO: implement minimax
// TODO: add logging, csv
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <ftxui-grid-container/grid-container.hpp>
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
#include <nlohmann/json.hpp>
#include <random>
#include <string>
#include <string_view>
#include <vector>
using nlohmann::json;

using json = json;

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

using GameStateGrid = std::array<std::string, GRIDSIZE>;
StateEnum checkWinner(const GameStateGrid& gameState) {
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

int bot(const GameStateGrid& gameState, StateEnum player) {
  // Check if AI can win in the next move.
  for (size_t i = 0; i < GRIDSIZE; ++i) {
    if (gameState[i] == State[idx(StateEnum::NONE)]) {
      GameStateGrid tempState = gameState;
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
      GameStateGrid tempState = gameState;
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
size_t findBestMove(const GameStateGrid& gameState) {
  return bot(gameState, StateEnum::O);  // AI player always plays 'O'
}
void buttonCallback(size_t index,
                    GameStateGrid& gameState,
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

Scores parseScoresFromFile(const std::filesystem::path& filePath) {
  if (!std::filesystem::exists(filePath)) {
    // If file doesn't exist, create and initialize it
    std::ofstream ofs(filePath);
    json data = {{"wins", {{"X", 0}, {"O", 0}}}};
    ofs << data.dump(4);  // Write formatted JSON to file
    return {0, 0};        // Return initialized scores
  }

  std::ifstream ifs(filePath);
  json data;
  ifs >> data;  // Read JSON from file

  return {data["wins"]["X"], data["wins"]["O"]};  // Extract and return scores
}

void writeScoresToFile(const std::filesystem::path& filePath,
                       const Scores& scores) {
  json data;
  if (std::filesystem::exists(filePath)) {
    std::ifstream ifs(filePath);
    ifs >> data;  // Read existing JSON from file
  } else {
    // If file doesn't exist, create an empty JSON object
    data = {{"wins", {{"X", 0}, {"O", 0}}}};
  }

  // Update scores in the JSON object
  data["wins"]["X"] = scores.x;
  data["wins"]["O"] = scores.o;

  std::ofstream ofs(filePath);
  ofs << data.dump(4);  // Write formatted JSON to file
}

int main() {
  auto screen = ScreenInteractive::TerminalOutput();
  StateEnum currentPlayer = StateEnum::X;
  std::string_view output = CurrentPlayer[idx(currentPlayer)];
  auto style = size(WIDTH, EQUAL, 8);
  Components buttons(GRIDSIZE);
  GameStateGrid gameState;
  Scores scores;
  gameState.fill(std::string(State[idx(StateEnum::NONE)]));
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
  const std::filesystem::path filePath{"wins.json"};

  scores = parseScoresFromFile(filePath);

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
  writeScoresToFile(filePath, scores);
  std::cout << "HEY";
  return 0;
}
