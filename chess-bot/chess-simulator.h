#pragma once
#include <string>
#include <random>
#include <utility>
#include "chess.hpp"

struct mctsNode{
    chess::Board board;
    chess::Move saveMove;
    float potential;
    mctsNode* parent;
    std::vector<chess::Move> openMoves;
    std::vector<chess::Move> foundMoves;

    mctsNode()
    {
        potential = std::numeric_limits<float>::infinity();
        parent = nullptr;

        foundMoves = std::vector<chess::Move>();
        openMoves = std::vector<chess::Move>();
        foundMoves.clear();
        openMoves.clear();
    }
    mctsNode(chess::Board& board, chess::Move& saveMove, float potential)
    {
        this->board = board;
        this->saveMove = saveMove;
        this->potential = potential;
        this->parent = nullptr;

        foundMoves = std::vector<chess::Move>();
        openMoves = std::vector<chess::Move>();
        foundMoves.clear();
        openMoves.clear();
    }

    mctsNode(chess::Board& board, float potential)
    {
        this->board = board;
        this->potential = potential;
        this->parent = nullptr;

        foundMoves = std::vector<chess::Move>();
        openMoves = std::vector<chess::Move>();
        foundMoves.clear();
        openMoves.clear();
    }

    int findIndexInFound(chess::Move move)
    {
        auto it = std::find(foundMoves.begin(), foundMoves.end(), move);
        int index;
        if(it != foundMoves.end())
        {
            index = std::distance(foundMoves.begin(), it);
        }
        else
            index =-1;

        return index;
    }
    int findIndexInOpen(chess::Move move)
    {
        auto it = std::find(openMoves.begin(), openMoves.end(), move);
        int index;
        if(it != openMoves.end())
        {
            index = std::distance(openMoves.begin(), it);
        }
        else
            index =-1;

        return index;
    }


    bool containsMove(chess::Move move)
    {
        int index = findIndexInFound(move);
        return index != -1;
    }

    double getUCB(double c)
    {
        if((double)foundMoves.size() > 0 && parent)
            return potential + 1 * sqrt(log((double)parent->foundMoves.size()/(double)foundMoves.size()));
        else
            return std::numeric_limits<double>::max();
    }

    void genOpenMoves()
    {
        openMoves.clear();

        chess::Movelist moves;
        chess::movegen::legalmoves(moves, board);

        for(int i = 0; i < moves.size(); i++)
        {
            openMoves.push_back(moves[i]);
        }
    }

    bool operator== (mctsNode rhs)
    {
        return saveMove == rhs.saveMove && potential == rhs.potential && parent == rhs.parent && openMoves == rhs.openMoves && foundMoves == rhs.foundMoves;
    }
};

namespace ChessSimulator {
/**
 * @brief Move a piece on the board
 *
 * @param fen The board as FEN
 * @return std::string The move as UCI
 */
std::string Move(std::string fen);
void Selection(chess::Board& board, std::vector<mctsNode>& nodes);
void Expansion(int nodeIndex, std::vector<mctsNode>& nodes);
float Simulation(chess::Board& board, chess::Color rootColor);
} // namespace ChessSimulator