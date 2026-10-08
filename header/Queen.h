#pragma once
#include "Piece.h"

namespace Chess {

    class Queen :
        public Piece
    {
    public:

        bool moves_like_rook = true;
        bool moves_like_bishop = true;

        Queen(const glm::vec3& position, const int64_t& board_id, const Piece::COLOR& color)
            : Piece(position, board_id, color, std::string("queen") + (color ? "_white" : "_black")) {};

        Queen() = default;


        //This needs to be in every WorldObject to deduce types for serialziation templates from polymorphism
        // Just change the template parameter to match your class
        int getTypeId(Registry* r) const {
            return r->getIdForType<Queen>();
        }

        bool isValidMove(const glm::vec3& destination) const override {
            if (!Piece::isValidMove(destination)) return false;

            return !blockedBy(destination) && (movedLikeRook(destination) || movedLikeBishop(destination));
        }
    };

    auto static getStructure(Queen& obj) {
        return std::tie(obj.position, obj.model_name, obj.color, obj.board_id, obj.has_moved, obj.moved_count, obj.last_moved_position, obj.last_moved_turn);
    };

}
