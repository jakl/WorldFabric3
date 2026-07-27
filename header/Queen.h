#pragma once
#include "Piece.h"

namespace Chess {

    class Queen :
        public Piece
    {
    public:

        Queen(const glm::vec3& position, Piece::COLOR color, int64_t board_id)
            : Piece(position, color, std::string("queen") + (color ? "_white" : "_black"), board_id) {};

        Queen() = default;


        //This needs to be in every WorldObject to deduce types for serialziation templates from polymorphism
        // Just change the template parameter to match your class
        int getTypeId(Registry* r) const {
            return r->getIdForType<Queen>();
        }

        bool isValidMove(const glm::vec2& source_square, const glm::vec2& destination_square) const override {
            bool moves_like_rook = source_square.x == destination_square.x || source_square.y == destination_square.y;
            bool moves_like_bishop = fabs(source_square.x - destination_square.x) - fabs(source_square.y - destination_square.y) == 0;
            return moves_like_rook || moves_like_bishop;
        }
    };

    auto static getStructure(Queen& obj) {
        return std::tie(obj.position, obj.model_name, obj.color);
    };

}
