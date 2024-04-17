#include "chess-simulator.h"
// disservin's lib. drop a star on his hard work!
// https://github.com/Disservin/chess-library
//#include "chess.hpp"
//#include <random>
#include <vector>
using namespace ChessSimulator;


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

void ChessSimulator::Selection(std::string fen, std::vector<mctsNode> nodes)
{
    if(!nodes.empty())
    {
        mctsNode* bestNode = nullptr;

        for(int i = 0; i < nodes.size(); i++)
        {
            if(bestNode == nullptr)
            {
                bestNode = &nodes[i];
            }
            else
            {
                if(bestNode->potential < nodes[i].potential)
                {
                    bestNode = &nodes[i];
                }
            }
        }

        Expansion(*bestNode, nodes);
    }
    else
    {
        //fill with first layer and apply initial potential

        chess::Board board(fen);
        chess::Movelist moves;
        chess::movegen::legalmoves(moves, board);

        mctsNode tempNode(fen, 0);
        tempNode.openMoves = moves;
    }
}

void ChessSimulator::Expansion(mctsNode node, std::vector<mctsNode> nodes)
{
    //first go at it! not tested
    chess::Board board(node.fen);

    std::random_device rd;
    bool foundNewMove = false;
    chess::Move move;
    while (!foundNewMove)
    {
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, node.openMoves.size() - 1);
        move = node.openMoves[dist(gen)];

        if(!node.containsMove(move))
        {
            foundNewMove = true;
        }
    }
    //std::string fen, float potential, mctsNode parent
        //below is assuming that uci is how you get fen
    mctsNode tempNode(chess::uci::moveToUci(move), 0, node);
    chess::Board tempBoard(tempNode.fen);
    chess::movegen::legalmoves(tempNode.openMoves, tempBoard);
    float potential = Simulation(tempNode.fen, board.sideToMove() == chess::Color::WHITE ? chess::Color::BLACK :chess::Color::WHITE);

    tempNode.potential += potential;

    mctsNode* backPropNode = tempNode.parent;
    while(backPropNode != nullptr)
    {
        backPropNode->potential += potential;
        backPropNode = backPropNode->parent;
    }

    nodes.push_back(tempNode);
    node.foundMoves.add(move);

    int moveIndex = node.openMoves.find(move);
    chess::Movelist tempMoves;

    if(moveIndex != -1)
    {
        for(int i = 0; i < node.openMoves.size(); i++)
        {
            if(i != moveIndex)
            {
                tempMoves.add(node.openMoves[i]);
            }
        }
    }
    node.openMoves.clear();
    node.openMoves.empty();
    node.openMoves = tempMoves;
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

             //below is assuming that uci is how you get fen
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
