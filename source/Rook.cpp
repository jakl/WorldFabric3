#include "Piece.h"
#include "Board.h"

#include "ChessApp.h"
#include <print>


namespace Chess {

    bool Rook::isValidMove(const glm::vec3& destination) const {
        if (!Piece::isValidMove(destination)) return false;

        return !blocked_by(destination) && moved_like_rook(destination);
    }

    void Rook::castle() {
        float speed = 2.0f;

        // Rook is on A1 or H8
        if (!!color == position.x < 0) speed = 3.0f;
        if (position.x > 0) speed *= -1.0f;

        queue(board_id, time, &Board::setPiecePosition, position, glm::vec3(position.x + speed, position.y, position.z));
    }
}
