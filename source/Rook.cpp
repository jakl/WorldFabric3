#include "Piece.h"
#include "Board.h"

#include "ChessApp.h"
#include <print>


namespace Chess {

    bool Rook::isValidMove(const glm::vec3& destination_square) const {
        WorldPlugin* world = getTool<WorldPlugin>();

        if (!Piece::isValidMove(destination_square)) return false;

        std::shared_ptr<const Board> board = world->observe<Board>("chess", board_id);

        if (!board) {
            return false;
        }

        for (const auto& square : squaresBetween(position, destination_square)) {
            auto maybe_piece = board->board_of_pieces.find(square);
            if (maybe_piece != board->board_of_pieces.end()) {
                // Piece in the way
                return false;
            }
        }

        return position.x == destination_square.x || position.z == destination_square.z;
    }
}