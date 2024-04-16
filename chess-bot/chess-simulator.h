#pragma once
#include <string>
#include <random>
#include <utility>
#include "chess.hpp"

struct mctsNode{
    std::string fen;
    float potential;
    mctsNode* parent;
    chess::Movelist kidMoves;

    mctsNode()
    {
        fen = "";
        potential = std::numeric_limits<float>::infinity();
        parent = nullptr;
    }
    mctsNode(std::string fen, float potential, mctsNode parent)
    {
        this->fen = std::move(fen);
        this->potential = potential;
        this->parent = &parent;
    }

    bool containsMove(chess::Move move)
    {
        return kidMoves.find(move) != -1;
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
void Selection(std::string fen, std::vector<mctsNode> nodes);
void Expansion(mctsNode node, std::vector<mctsNode> nodes);
float Simulation(std::string fen, chess::Color rootColor);
} // namespace ChessSimulator