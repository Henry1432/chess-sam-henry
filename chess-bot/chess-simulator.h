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
    chess::Movelist openMoves;
    chess::Movelist foundMoves;

    mctsNode()
    {
        potential = std::numeric_limits<float>::infinity();
        parent = nullptr;
    }
    mctsNode(chess::Board board, chess::Move saveMove, float potential, mctsNode parent)
    {
        this->board = board;
        this->saveMove = saveMove;
        this->potential = potential;
        this->parent = &parent;
    }

    mctsNode(chess::Board board, float potential)
    {
        this->board = board;
        this->potential = potential;
        this->parent = nullptr;
    }

    bool containsMove(chess::Move move)
    {
        return foundMoves.find(move) != -1;
    }

    double getUCB(double c)
    {
        if((double)foundMoves.size() > 0)
            return potential + 1 * sqrt(log((double)parent->foundMoves.size()/(double)foundMoves.size()));
        else
            return std::numeric_limits<double>::max();
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
void Selection(chess::Board board, std::vector<mctsNode> nodes);
void Expansion(mctsNode node, std::vector<mctsNode> nodes);
float Simulation(chess::Board board, chess::Color rootColor);
} // namespace ChessSimulator