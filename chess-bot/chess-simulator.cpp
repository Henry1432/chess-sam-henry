#include "chess-simulator.h"
// disservin's lib. drop a star on his hard work!
// https://github.com/Disservin/chess-library
//#include "chess.hpp"
//#include <random>
#include <vector>
using namespace ChessSimulator;

struct mctsNode
{
    std::string fen;
    float potential;
};

std::string ChessSimulator::Move(std::string fen) {
    int const NUM_SIM = 1000;
  // create your board based on the board string following the FEN notation
  // search for the best move using minimax / monte carlo tree search /
  // alpha-beta pruning / ... try to use nice heuristics to speed up the search
  // and have better results return the best move in UCI notation you will gain
  // extra points if you create your own board/move representation instead of
  // using the one provided by the library

  // here goes a random movement
  chess::Board board(fen);
  chess::Movelist moves;
  chess::movegen::legalmoves(moves, board);
  if(moves.size() == 0)
    return "";

  std::vector<int> eval;

  for(int i =0; i < NUM_SIM; i++)
  {

  }

  // get random move
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dist(0, moves.size() - 1);
  auto move = moves[dist(gen)];
  return chess::uci::moveToUci(move);
}

void ChessSimulator::Selection(std::string fen)
{

}

void ChessSimulator::Expansion(std::string fen)
{

}

float ChessSimulator::Simulation(std::string fen, chess::Color rootColor)
{
    //Checks who wins and returns correct value
    chess::Board board(fen);
    chess::GameResult gameState = board.isGameOver().second;
    if(gameState == chess::GameResult::NONE)
    {
        chess::Movelist moves;
        chess::movegen::legalmoves(moves, board);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, moves.size() - 1);
        auto move = moves[dist(gen)];

        return Simulation(chess::uci::moveToUci(move), rootColor);
    }
    else if (gameState == chess::GameResult::DRAW)
    {
        return -0.5;
    }
    else
    {
        chess::Color attackColor = rootColor == chess::Color::WHITE ? chess::Color::BLACK : chess::Color::WHITE;
        if(board.isAttacked(board.kingSq(rootColor), attackColor))
        {
            return -1;
        }
        return 1;
    }
}

//steps each being its own function
    //simulate all layer 1 moves
    //loop this move until we run out of time
        //find board with most potential
        // simulate down with random moves
        //send win state back up the tree and modify potential values
    //send move
