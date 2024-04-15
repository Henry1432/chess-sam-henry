#pragma once
#include <string>
#include <random>
#include "chess.hpp"

namespace ChessSimulator {
/**
 * @brief Move a piece on the board
 *
 * @param fen The board as FEN
 * @return std::string The move as UCI
 */
std::string Move(std::string fen);
void Selection(std::string fen)
void Expansion(std::string fen);
float Simulation(std::string fen, chess::Color rootColor);
} // namespace ChessSimulator