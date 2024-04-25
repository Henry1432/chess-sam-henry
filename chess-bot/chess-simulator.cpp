#include "chess-simulator.h"
#include <chrono>
// disservin's lib. drop a star on his hard work!
// https://github.com/Disservin/chess-library
//#include "chess.hpp"
//#include <random>
#include <vector>
#include <cmath>
using namespace ChessSimulator;

std::string ChessSimulator::Move(std::string fen) {
    int const NUM_SIM = 25000;
    int r = 0;
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
  auto timer = std::chrono::system_clock::now();
  int count = 0;
  while(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - timer).count() < 9989)
  {
      Selection(board, nodes, r);
      count++;
  }
  //std::cout<<count << "\t";
  //std::cout<<r<<std::endl;

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
void ChessSimulator::Selection(chess::Board& board, std::vector<mctsNode>& nodes, int& r)
{
    if(!nodes.empty())
    {
        //selection process is da problem! fix it pweaz :(
        std::vector<int> bestNode;
        double UCB;

        for(int i = 0; i < nodes.size(); i++)
        {
            if(bestNode.size() == 0)
            {
                bestNode.push_back(i);
                UCB = nodes[bestNode[0]].getUCB(2, nodes);
            }
            else if (nodes[i].openMoves.size() > 0)
            {
                double tempUCB = nodes[i].getUCB(2, nodes);

                if(tempUCB == UCB)
                {
                    bestNode.push_back(i);
                }
                else if(UCB < tempUCB)
                {
                    bestNode.clear();
                    bestNode.push_back(i);
                    UCB = nodes[bestNode[0]].getUCB(2, nodes);
                }
            }
        }

        int randIndex = bestNode.size()/2;
        Expansion(bestNode[randIndex], nodes, r);

        //std::cout << duration_cast< std::chrono::milliseconds >(std::chrono::system_clock::now().time_since_epoch()) << std::endl;
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
}*/

void ChessSimulator::Selection(chess::Board& board, std::vector<mctsNode>& nodes) {
    // If the tree is empty, fill it with the first layer and apply initial potential
    if (nodes.empty()) {
        chess::Movelist moves;
        chess::movegen::legalmoves(moves, board);

        for (int i = 0; i < moves.size(); ++i) {
            mctsNode tempNode(board, moves[i], 0);
            tempNode.genOpenMoves();
            float potential = Simulation(board, board.sideToMove());
            tempNode.potential += potential;
            nodes.push_back(tempNode);
        }
    } else {
        // Traverse the tree until a leaf node is reached
        mctsNode* currentNode = &nodes[0];
        int nodeIndex = 0;
        while (!currentNode->openMoves.empty()) {
            double maxUCB = -std::numeric_limits<double>::infinity();
            int bestChildIndex = -1;

            // Calculate UCB for each child node and select the one with the highest UCB
            for (int i = 0; i < currentNode->openMoves.size(); ++i) {
                double UCB = currentNode->getUCB(i, nodes);
                if (UCB > maxUCB) {
                    maxUCB = UCB;
                    bestChildIndex = i;
                }
            }

            // If all children have been visited, expand the node
            if (bestChildIndex == -1) {
                Expansion(nodeIndex, nodes);
                // Select one of the newly expanded nodes
                bestChildIndex = currentNode->children.size() - 1;
            }

            // Move to the selected child node
            nodeIndex = currentNode->children[bestChildIndex].index;
            currentNode = &nodes[nodeIndex];
        }
    }
}

void ChessSimulator::Expansion(int nodeIndex, std::vector<mctsNode>& nodes, int& r)
{
    //first go at it! not tested
    if(nodeIndex >= 0)
    {
        if(nodes[nodeIndex].openMoves.size() > 0)
        {
            //std::random_device rd; //commented for open tasting
            bool foundNewMove = true;//, loop = true; //commented for open tasting
            chess::Move move;
            //int saveIndex;//commented for open tasting
            //int counter = 0;//commented for open tasting

            move = nodes[nodeIndex].openMoves[nodes[nodeIndex].openMoves.size()-1];


            /*
             * testing standardized open picking
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
             */
            //if(foundNewMove)
            //{
            chess::Board tempBoard(nodes[nodeIndex].board);
            tempBoard.makeMove(move);
            mctsNode tempNode(tempBoard, move, 0);

            tempNode.genOpenMoves();
            float potential = Simulation(tempBoard, tempBoard.sideToMove(), r);
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
            nodes[nodeIndex].openMoves.erase(nodes[nodeIndex].openMoves.end());
            //nodes[nodeIndex].openMoves.erase(nodes[nodeIndex].openMoves.begin() + saveIndex);
            nodes.push_back(tempNode);
            //nodes[nodes.size() - 1].parent = &nodes[nodeIndex];
            nodes[nodes.size() - 1].parentIndex = nodeIndex;
            //}
        }
    }
}

float ChessSimulator::Simulation(chess::Board& board, chess::Color rootColor, int& r)
{
    r++;
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
