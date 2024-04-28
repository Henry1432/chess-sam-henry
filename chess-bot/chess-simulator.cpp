#include "chess-simulator.h"
#include <chrono>
// disservin's lib. drop a star on his hard work!
// https://github.com/Disservin/chess-library
//#include "chess.hpp"
//#include <random>
#include <vector>
#include <cmath>
using namespace ChessSimulator;

int ChessSimulator::getRandNum(int seed, int min, int max)
{
    int value = 0;
    for (int x = 0; x < 100; x++)
    {
        seed ^= seed << 13;
        seed ^= seed >> 17;
        seed ^= seed << 5;
        value = min + (seed % (max));
    }

    return abs(value);
}

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
  while(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - timer).count() < 9800)
  {
      try
      {
        Selection(board, nodes);
        count++;
      }
      catch(...)
      {
          //std::cout<<"selection fail\n";
      }
  }

    mctsNode* pickNode = &nodes[1];
    mctsNode* backupNode = &nodes[2];


    for(int i = 1; i < nodes.size(); i++)
    {
        if(pickNode->potential < nodes[i].potential && nodes[i].parentIndex != -1)
        {
            backupNode = pickNode;
            pickNode = &nodes[i];
            if(backupNode->parentIndex == -1)
            {
                backupNode = pickNode;
            }
        }
    }
    while(nodes[pickNode->parentIndex].parentIndex != -1)
    {
        if(pickNode->parentIndex < nodes.size())
        {
            pickNode = &nodes[pickNode->parentIndex];
        }
    }
    count = 0;
    while(nodes[backupNode->parentIndex].parentIndex != -1)
    {
        if(backupNode->parentIndex < nodes.size())
        {
            backupNode = &nodes[backupNode->parentIndex];
        }
    }

    if(board.at(pickNode->saveMove.from()).color() == board.sideToMove())
    {
        return chess::uci::moveToUci(pickNode->saveMove);
    }
    else
        return chess::uci::moveToUci(backupNode->saveMove);

}

//fen?
void ChessSimulator::Selection(chess::Board& board, std::vector<mctsNode>& nodes)
{
    if(!nodes.empty())
    {
        //selection process is da problem! fix it pweaz :(
        std::vector<int> bestNode;
        double UCB;

        for(int i = 0; i < nodes.size(); i++)
        {
            double tempUCB = nodes[i].getUCB(2, nodes);

            if(bestNode.size() == 0)
            {
                bestNode.push_back(i);
                UCB = tempUCB;
            }
            else if (nodes[i].openMoves.size() > 0)
            {
                if(tempUCB == UCB)
                {
                    bestNode.push_back(i);
                }
                else if(UCB < tempUCB)
                {
                    bestNode.clear();
                    bestNode.push_back(i);
                    UCB = tempUCB;
                }
            }
        }
        int now = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        int seed = (now ^ 31) % now/2.8914573;
        int randIndex = ChessSimulator::getRandNum(seed, 0, bestNode.size());
        try
        {
            if(randIndex < nodes.size())
                Expansion(bestNode[randIndex], nodes);
        }
        catch(...)
        {
            //std::cout << "expansion failed\n";
        }
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
}

void ChessSimulator::Expansion(int nodeIndex, std::vector<mctsNode>& nodes)
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
            float potential = Simulation(nodes, nodeIndex, tempBoard, tempBoard.sideToMove());
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

float ChessSimulator::Simulation(std::vector<mctsNode>& nodes, int nodeIndex, chess::Board& board, chess::Color rootColor)
{
    float h = HTest(nodes, nodeIndex, board, rootColor);
    while(board.isGameOver().second == chess::GameResult::NONE)
    {
        chess::Movelist moves;
        chess::movegen::legalmoves(moves, board);
        int now = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        int seed = (now ^ 31) % now/2.8914573;
        int randIndex = ChessSimulator::getRandNum(seed, 0, moves.size());

        auto move = moves[randIndex];
        board.makeMove(move);
    }
    if (board.isGameOver().second == chess::GameResult::DRAW)
    {
        return h;
    }
    else
    {
        chess::Color attackColor = rootColor == chess::Color::WHITE ? chess::Color::BLACK : chess::Color::WHITE;
        if(board.isAttacked(board.kingSq(rootColor), attackColor))
        {
            return -5 + h;
        }
        return 5 + h;
    }
}

float ChessSimulator::HTest(std::vector<mctsNode>& nodes, int nodeIndex, chess::Board &board, chess::Color color)
{
    //board.pieces(chess::PieceType::KING, color).lsb();
    float value = 0;
    int pPieceCount = 0, ePieceCount = 0;

    if(board.pieces(chess::PieceType::PAWN, color) != chess::Bitboard(0))
    {
        chess::Square sPawn = board.pieces(chess::PieceType::PAWN, color).lsb();
        pPieceCount += board.pieces(chess::PieceType::PAWN, color).count();
        if(board.isAttacked(sPawn, ~color))
        {
            value -= 0.2f;
        }
    }
    if(board.pieces(chess::PieceType::KNIGHT, color) != chess::Bitboard(0))
    {
        chess::Square sKnight = board.pieces(chess::PieceType::KNIGHT, color).lsb();
        pPieceCount += board.pieces(chess::PieceType::KNIGHT, color).count();
        if(board.isAttacked(sKnight, ~color))
        {
            value -= 0.4f;
        }
    }
    if(board.pieces(chess::PieceType::BISHOP, color) != chess::Bitboard(0))
    {
        chess::Square sBish = board.pieces(chess::PieceType::BISHOP, color).lsb();
        pPieceCount += board.pieces(chess::PieceType::BISHOP, color).count();
        if(board.isAttacked(sBish, ~color))
        {
            value -= 0.4f;
        }
    }
    if(board.pieces(chess::PieceType::ROOK, color) != chess::Bitboard(0))
    {
        chess::Square sRook = board.pieces(chess::PieceType::ROOK, color).lsb();
        pPieceCount += board.pieces(chess::PieceType::ROOK, color).count();
        if(board.isAttacked(sRook, ~color))
        {
            value -= 0.4f;
        }
    }
    if(board.pieces(chess::PieceType::QUEEN, color) != chess::Bitboard(0))
    {
        chess::Square sQueen = board.pieces(chess::PieceType::QUEEN, color).lsb();
        pPieceCount += board.pieces(chess::PieceType::QUEEN, color).count();
        if(board.isAttacked(sQueen, ~color))
        {
            value -= 0.6f;
        }
    }


    if(board.pieces(chess::PieceType::PAWN, ~color) != chess::Bitboard(0))
    {
        chess::Square sPawn = board.pieces(chess::PieceType::PAWN, ~color).lsb();
        ePieceCount += board.pieces(chess::PieceType::PAWN, ~color).count();
        if(board.isAttacked(sPawn, color))
        {
            value += 0.1f;
        }
    }
    if(board.pieces(chess::PieceType::KNIGHT, ~color) != chess::Bitboard(0))
    {
        chess::Square sKnight = board.pieces(chess::PieceType::KNIGHT, ~color).lsb();
        ePieceCount += board.pieces(chess::PieceType::KNIGHT, ~color).count();
        if(board.isAttacked(sKnight, color))
        {
            value += 0.3f;
        }
    }
    if(board.pieces(chess::PieceType::BISHOP, ~color) != chess::Bitboard(0))
    {
        chess::Square sBish = board.pieces(chess::PieceType::BISHOP, ~color).lsb();
        ePieceCount += board.pieces(chess::PieceType::BISHOP, ~color).count();
        if(board.isAttacked(sBish, color))
        {
            value += 0.3f;
        }
    }
    if(board.pieces(chess::PieceType::ROOK, ~color) != chess::Bitboard(0))
    {
        chess::Square sRook = board.pieces(chess::PieceType::ROOK, ~color).lsb();
        ePieceCount += board.pieces(chess::PieceType::ROOK, ~color).count();
        if(board.isAttacked(sRook, color))
        {
            value += 0.4f;
        }
    }
    if(board.pieces(chess::PieceType::QUEEN, ~color) != chess::Bitboard(0))
    {
        chess::Square sQueen = board.pieces(chess::PieceType::QUEEN, ~color).lsb();
        ePieceCount += board.pieces(chess::PieceType::QUEEN, ~color).count();
        if(board.isAttacked(sQueen, color))
        {
            value += 0.6f;
        }
    }
    if((nodeIndex < nodes.size() && nodeIndex != -1) && (nodes[nodeIndex].parentIndex < nodes.size() && nodes[nodeIndex].parentIndex != -1)) {
        int ppPieceCount = 0, pePieceCount = 0;
        ppPieceCount += nodes[nodes[nodeIndex].parentIndex].board.pieces(chess::PieceType::PAWN, color).count();
        ppPieceCount += nodes[nodes[nodeIndex].parentIndex].board.pieces(chess::PieceType::KNIGHT, color).count();
        ppPieceCount += nodes[nodes[nodeIndex].parentIndex].board.pieces(chess::PieceType::BISHOP, color).count();
        ppPieceCount += nodes[nodes[nodeIndex].parentIndex].board.pieces(chess::PieceType::ROOK, color).count();
        ppPieceCount += nodes[nodes[nodeIndex].parentIndex].board.pieces(chess::PieceType::QUEEN, color).count();

        pePieceCount += nodes[nodes[nodeIndex].parentIndex].board.pieces(chess::PieceType::PAWN, ~color).count();
        pePieceCount += nodes[nodes[nodeIndex].parentIndex].board.pieces(chess::PieceType::KNIGHT, ~color).count();
        pePieceCount += nodes[nodes[nodeIndex].parentIndex].board.pieces(chess::PieceType::BISHOP, ~color).count();
        pePieceCount += nodes[nodes[nodeIndex].parentIndex].board.pieces(chess::PieceType::ROOK, ~color).count();
        pePieceCount += nodes[nodes[nodeIndex].parentIndex].board.pieces(chess::PieceType::QUEEN, ~color).count();

        if (ppPieceCount > pPieceCount) {
            value -= 3;
        }
        if (pePieceCount > ePieceCount) {
            value += 3;
        }
    }
    return value;
}
//steps each being its own function
    //simulate all layer 1 moves
    //loop this move until we run out of time
        //find board with most potential
        // simulate down with random moves
        //send win state back up the tree and modify potential values
    //send move
