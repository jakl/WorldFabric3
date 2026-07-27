#pragma once
#include "Piece.h"

namespace Chess {

    class Knight :
        public Piece
    {
    public:

        Knight(const glm::vec3& position, Piece::COLOR color, int64_t board_id)
            : Piece(position, color, std::string("knight") + (color ? "_white" : "_black"), board_id) {};

        Knight() = default;


        //This needs to be in every WorldObject to deduce types for serialziation templates from polymorphism
        // Just change the template parameter to match your class
        int getTypeId(Registry* r) const {
            return r->getIdForType<Knight>();
        }

        bool isValidMove(const glm::vec2& source_square, const glm::vec2& destination_square) const override {
            return fabs(source_square.x - destination_square.x) == 2 && fabs(source_square.y - destination_square.y) == 1
                || fabs(source_square.x - destination_square.x) == 1 && fabs(source_square.y - destination_square.y) == 2;
        }
    };

    auto static getStructure(Knight& obj) {
        return std::tie(obj.position, obj.model_name, obj.color);
    };

}
