#include "chess-simulator.h"
// disservin's lib. drop a star on his hard work!
// https://github.com/Disservin/chess-library
//#include "chess.hpp"
//#include <random>
#include <vector>
#include <cmath>
using namespace ChessSimulator;


std::string ChessSimulator::Move(std::string fen) {
    int const NUM_SIM = 500; //I am not sure what, but something is happening after 200 simulations, next step is optomization, it is agonizingly slow,
                                // the problem could be some timing out thing because of how slow it is,
                                    // might be a deeper issue but that would be more complex and Im hoping I can fix it by being a more efficient program.
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
  std::vector<mctsNode> nodes;

  for(int i =0; i < NUM_SIM; i++)
  {
      Selection(board, nodes);
  }

  mctsNode* pickNode = &nodes[0];

  for(int i = 1; i < nodes.size(); i++)
  {
      if(pickNode->potential < nodes[i].potential)
      {
          pickNode = &nodes[i];
      }
  }

  while(nodes[pickNode->parentIndex].parentIndex != -1)
  {
      pickNode = &nodes[pickNode->parentIndex];
  }

  return chess::uci::moveToUci(pickNode->saveMove);
}

//fen?
void ChessSimulator::Selection(chess::Board& board, std::vector<mctsNode>& nodes)
{
    if(!nodes.empty())
    {
        std::vector<mctsNode*> bestNode;
        double UCB;

        for(int i = 0; i < nodes.size(); i++)
        {
            if(bestNode.size() == 0)
            {
                bestNode.push_back(&nodes[i]);
                UCB = bestNode[0]->getUCB(2, nodes);
            }
            else
            {
                double tempUCB = nodes[i].getUCB(2, nodes);

                if(tempUCB == UCB)
                {
                    bestNode.push_back(&nodes[i]);
                }
                else if(UCB < tempUCB)
                {
                    bestNode.clear();
                    bestNode.push_back(&nodes[i]);
                    UCB = bestNode[0]->getUCB(2, nodes);
                }
            }
        }
        int index = 0;
        if(bestNode.size() > 1)
        {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> bestIndex(0, bestNode.size() - 1);
            index = bestIndex(gen);
        }

        auto it = std::find(nodes.begin(), nodes.end(), *bestNode[index]);
        int nodeIndex;
        if(it != nodes.end())
        {
            nodeIndex = std::distance(nodes.begin(), it);
        }
        else
            nodeIndex =-1;

        Expansion(nodeIndex, nodes);
    }
    else
    {
        //fill with first layer and apply initial potential

        chess::Movelist moves;
        chess::movegen::legalmoves(moves, board);

        mctsNode tempNode(board, 0);
        for(int i = 0; i < moves.size(); i++)
        {
            tempNode.openMoves.push_back(moves[i]);
        }

        nodes.push_back(tempNode);
    }
}

void ChessSimulator::Expansion(int nodeIndex, std::vector<mctsNode>& nodes)
{
    //first go at it! not tested
    if(nodeIndex >= 0)
    {
        if(nodes[nodeIndex].openMoves.size() > 0)
        {
            std::random_device rd;
            bool foundNewMove = false, loop = true;
            chess::Move move;
            int saveIndex;
            int counter = 0;
            //should only run once I believe
            while (loop)
            {
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dist(0, nodes[nodeIndex].openMoves.size() - 1);
                saveIndex = dist(gen);
                move = nodes[nodeIndex].openMoves[saveIndex];

                if(!nodes[nodeIndex].containsMove(move))
                {
                    foundNewMove = true;
                }

                if(counter > 112 || foundNewMove)
                {
                    loop = false;
                }
            }
            if(foundNewMove)
            {
                chess::Board tempBoard(nodes[nodeIndex].board);
                tempBoard.makeMove(move);
                mctsNode tempNode(tempBoard, move, 0);

                tempNode.genOpenMoves();
                float potential = Simulation(tempBoard, tempBoard.sideToMove());
                        //Simulation(tempBoard, tempBoard.sideToMove());

                tempNode.potential += potential;

                mctsNode* backPropNode = &nodes[nodeIndex];
                while(backPropNode != nullptr)
                {
                    backPropNode->potential += potential;
                    if(backPropNode->parentIndex != -1)
                    {
                        backPropNode = &nodes[backPropNode->parentIndex];
                    }
                    else
                    {
                        backPropNode = nullptr;
                    }
                }

                nodes[nodeIndex].foundMoves.push_back(move);
                nodes[nodeIndex].openMoves.erase(nodes[nodeIndex].openMoves.begin() + saveIndex);
                nodes.push_back(tempNode);
                //nodes[nodes.size() - 1].parent = &nodes[nodeIndex];
                nodes[nodes.size() - 1].parentIndex = nodeIndex;
            }
        }
    }
}

float ChessSimulator::Simulation(chess::Board& board, chess::Color rootColor)
{
    while(board.isGameOver().second == chess::GameResult::NONE)
    {
        chess::Movelist moves;
        chess::movegen::legalmoves(moves, board);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, moves.size() - 1);
        auto move = moves[dist(gen)];
        board.makeMove(move);
    }
    if (board.isGameOver().second == chess::GameResult::DRAW)
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
