#include <algorithm>
#include <array>
#include <cassert>
#include <climits>
#include <iostream>
#include <stack>
#include <string>
#include <vector>

#define INTERACTIBLE true

using namespace std;

enum GridSquare : char
{
  x = 'X',
  o = 'O',
  blank = ' ',
};
enum GameState : char
{
  x_won = x,
  o_won = o,
  ongoing,
  draw,
};

class TicTacToeBoard
{
public:
  TicTacToeBoard() : board(), moves(), x_to_play(true) { board.fill(blank); }

  struct Move
  {
    int x, y;
  };

  GridSquare &at(int x, int y)
  {
    return board[3 * y + x];
  }
  GridSquare at(int x, int y) const
  {
    return board[3 * y + x];
  }

  void clear()
  {
    board.fill(blank);
    while (!moves.empty())
      moves.pop();
  }

  GridSquare getNextPlayer() const
  {
    return (x_to_play) ? x : o;
  }
  GridSquare getPrevPlayer() const
  {
    return (x_to_play) ? o : x;
  }

  vector<Move> getMoves() const
  {
    // cerr << "generating moves";
    if (gameState() != ongoing)
      return {};
    // cerr << '.';
    GridSquare p = getNextPlayer();
    vector<Move> ret;
    ret.reserve(5);
    for (int i = 0; i < board.size(); ++i)
    {
      // cerr << '.';
      if (board[i] == blank)
        ret.push_back({i % 3, i / 3});
    }
    // cerr << endl;
    return ret;
  }

  GameState gameState() const
  {
    // cerr << "Checking game state:\n";
    // cerr << "\nChecking verticals";
    for (int x = 0; x < 3; ++x)
    {
      // cerr << '.';
      //  vertial line
      auto top = at(x, 0);
      if (top != blank && top == at(x, 1) && top == at(x, 2))
      {
        return static_cast<GameState>(top);
      }
    }
    // cerr << "\nChecking horizontals";
    for (int y = 0; y < 3; ++y)
    {
      // cerr << '.';
      //  horizontal line
      if (at(0, y) != blank && at(0, y) == at(1, y) && at(1, y) == at(2, y))
      {
        return static_cast<GameState>(at(0, y));
      }
    }
    // cerr << "\nChecking diagonals";
    //  diagonals
    // cerr << '.';
    if (at(0, 0) != blank && at(0, 0) == at(1, 1) && at(1, 1) == at(2, 2))
    {
      return static_cast<GameState>(at(0, 0));
    }
    // cerr << '.';
    if (at(2, 0) != blank && at(2, 0) == at(1, 1) && at(1, 1) == at(0, 2))
    {
      return static_cast<GameState>(at(2, 0));
    }
    return (moves.size() == 9) ? draw : ongoing;
  }

  bool pushMove(const Move &move)
  {
    GridSquare &sq = at(move.x, move.y);
    if (sq != blank)
      return false;
    moves.push(move);
    sq = getNextPlayer();
    x_to_play = !x_to_play;
    return true;
  }
  void popMove()
  {
    Move move = moves.top();
    moves.pop();
    at(move.x, move.y) = blank;
    x_to_play = !x_to_play;
  }

  friend ostream &operator<<(ostream &os, TicTacToeBoard &board)
  {
    os << "\n+---+---+---+\n";
    for (int y = 0; y < 3; ++y)
    {
      os << "| ";
      for (int x = 0; x < 3; ++x)
      {
        os << static_cast<char>(board.at(x, y)) << " | ";
      }
      os << "\n+---+---+---+\n";
    }
    return os;
  }

private:
  array<GridSquare, 9> board;
  stack<Move> moves;
  bool x_to_play;
};

int negamax(TicTacToeBoard &board, int depth = 0)
{
  // cerr << "depth " << depth << endl;
  if (depth == 8)
    cerr << board << endl
         << endl;
  GameState state = board.gameState();
  switch (state)
  {
  case draw:
    return 0;
  case x_won:
  case o_won:
    return -1;
  case ongoing:
    break;
  }
  TicTacToeBoard::Move best;
  int best_score = -2;
  for (const auto &move : board.getMoves())
  {
    board.pushMove(move);
    int score = -negamax(board, depth + 1);
    if (score > best_score)
    {
      best = move;
      best_score = score;
    }
    board.popMove();
  }
  return best_score;
}

TicTacToeBoard::Move bestMove(TicTacToeBoard &board)
{
  if (board.getMoves().size() == 9)
    return {1, 1};
  TicTacToeBoard::Move best;
  int best_score = -2;
  for (const auto &move : board.getMoves())
  {
    board.pushMove(move);
    int score = -negamax(board);
    if (score > best_score)
    {
      best = move;
      best_score = score;
    }
    board.popMove();
  }
  return best;
}

#ifdef INTERACTIBLE

#define quit()              \
  do                        \
  {                         \
    cout << "Bye!" << endl; \
    exit(0);                \
  } while (0)

enum MenuOption
{
  play_as_x,
  play_as_o,
  quit
};
MenuOption menu()
{
  cout << "\n-- Tic Tac Toe --\n\n"
       << "Menu options:\n"
       << "  1: play as X (also 'x' or 'X')\n"
       << "  2: play as O (also 'o' or 'O')\n"
       << "  3: quit      (also 'q' or 'Q')\n"
       << "What would you like to do?\n"
       << ">> " << flush;
  while (true)
  {
    char selection;
    cin >> selection;
    cin.ignore();
    if (!cin.good())
      quit();
    switch (selection)
    {
    case '1':
    case 'x':
    case 'X':
      return play_as_x;
    case '2':
    case 'o':
    case 'O':
      return play_as_o;
    case '3':
    case 'q':
    case 'Q':
      quit();
    default:
      cout << "Invalid input, please try again.\n>> " << flush;
      break;
    }
  }
}

TicTacToeBoard::Move getUserMove()
{
  int row;
  int column;
  cout << "Where would you like to mark?\n";
  while (true)
  {
    cout << "Row: " << flush;
    cin >> row;
    cin.ignore();
    if (!cin.good())
    {
      if (cin.eof())
        quit();
      cout << "Invalid input, please try again.\n>> " << flush;
      cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      cin.clear();
      continue;
    }
    if (0 <= row && row < 3)
      break;
  }
  while (true)
  {
    cout << "Column: " << flush;
    cin >> column;
    cin.ignore();
    if (!cin.good())
    {
      if (cin.eof())
        return {-1, -1};
      cout << "Invalid input, please try again.\n>> " << flush;
      cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
      cin.clear();
      continue;
    }
    if (0 <= column && column < 3)
      break;
  }
  return {row, column};
}

int main()
{
  TicTacToeBoard board;
  while (true)
  {
    board.clear();
    switch (menu())
    {
    case quit:
      quit();
    case play_as_o:
      board.pushMove(bestMove(board));
      break;
    case play_as_x:
      break;
    }
    bool user_satisfied;
    auto moves = board.getMoves();
    while (true)
    {
      TicTacToeBoard::Move move;
      cout << board;
      while (true)
      {
        move = getUserMove();
        if (board.at(move.x, move.y) == blank)
          break;
        else
          cout << "That square is taken, please try again." << endl;
      }
      board.pushMove(move);
      cout << "Are you sure you want to make this move? (Y/N)\n>> " << flush;
      while (true)
      {
        char selection;
        cin >> selection;
        cin.ignore();
        if (!cin.good())
          quit();
        switch (selection)
        {
        case 'y':
        case 'Y':
          user_satisfied = true;
          break;
        case 'n':
        case 'N':
          user_satisfied = false;
          break;
        default:
          cout << "Invalid selection. Please try again.\n>> " << flush;
          continue;
        }
        break;
      }
      if (!user_satisfied)
        board.popMove();
      else
        board.pushMove(bestMove(board));
    }
  }
  return 1;
}

#else

int main()
{
  TicTacToeBoard board;

  // game loop
  while (1)
  {
    int opponent_row;
    int opponent_col;
    cin >> opponent_row >> opponent_col;
    cin.ignore();
    cerr << opponent_row << ' ' << opponent_col;
    if (opponent_row != -1 && opponent_col != -1)
    {
      board.pushMove({opponent_col, opponent_row});
    }
    cerr << board << endl;
    int valid_action_count;
    cin >> valid_action_count;
    cin.ignore();
    cerr << valid_action_count << " should == ";
    cerr << board.getMoves().size() << endl;
    for (int i = 0; i < valid_action_count; i++)
    {
      int row;
      int col;
      cin >> row >> col;
      cin.ignore();
    }

    // Write an action using cout. DON'T FORGET THE "<< endl"
    // To debug: cerr << "Debug messages..." << endl;
    auto best_move = bestMove(board);
    board.pushMove(best_move);
    cout << best_move.y << ' ' << best_move.x << endl;
  }
}

#endif