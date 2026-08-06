#include "Piece.h"
#include "Board.h"

#include "ChessApp.h"
#include <print>


namespace Chess {

    bool Rook::isValidMove(const glm::vec3& destination) const {
        if (!Piece::isValidMove(destination)) return false;

        return !blocked(destination) && moved_like_rook(destination);
    }
}