#pragma once
#include "Piece.h"

namespace Chess {

    class Rook :
        public Piece
    {
    public:

        Rook(const glm::vec3& position, Piece::COLOR color, int64_t board_id)
            : Piece(position, color, std::string("rook") + (color ? "_white" : "_black"), board_id) {};

        Rook() = default;


        //This needs to be in every WorldObject to deduce types for serialziation templates from polymorphism
        // Just change the template parameter to match your class
        int getTypeId(Registry* r) const {
            return r->getIdForType<Rook>();
        }

        bool isValidMove(const glm::vec2& source_square, const glm::vec2& destination_square) const override {
            return source_square.x == destination_square.x || source_square.y == destination_square.y;
        }
    };

    auto static getStructure(Rook& obj) {
        return std::tie(obj.position, obj.model_name, obj.color);
    };

}
