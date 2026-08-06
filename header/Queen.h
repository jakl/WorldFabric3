#pragma once
#include "Piece.h"

namespace Chess {

    class Queen :
        public Piece
    {
    public:

        Queen(const glm::vec3& position, const int64_t& board_id, const Piece::COLOR& color)
            : Piece(position, board_id, color, std::string("queen") + (color ? "_white" : "_black")) {};

        Queen() = default;


        //This needs to be in every WorldObject to deduce types for serialziation templates from polymorphism
        // Just change the template parameter to match your class
        int getTypeId(Registry* r) const {
            return r->getIdForType<Queen>();
        }

        bool isValidMove(const glm::vec3& destination_square) const override {
            if (!Piece::isValidMove(destination_square)) { return false; }
            bool moves_like_rook = position.x == destination_square.x || position.z == destination_square.z;
            bool moves_like_bishop = fabs(position.x - destination_square.x) - fabs(position.z - destination_square.z) == 0;
            return moves_like_rook || moves_like_bishop;
        }
    };

    auto static getStructure(Queen& obj) {
        return std::tie(obj.position, obj.model_name, obj.color, obj.board_id, obj.has_moved);
    };

}
