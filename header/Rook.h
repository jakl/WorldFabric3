#pragma once
#include "Piece.h"

namespace Chess {

    class Rook :
        public Piece
    {
    public:

        Rook(const glm::vec3& position, bool is_white)
            : Piece(position, is_white, "rook" + is_white ? "_white" : "_black") {};

        Rook() = default;


        //This needs to be in every WorldObject to deduce types for serialziation templates from polymorphism
        // Just change the template parameter to match your class
        int getTypeId(Registry* r) const {
            return r->getIdForType<Rook>();
        }

        bool isValidMove(const glm::vec2& source_square, const glm::vec2& destination_square) const override {
            if (source_square.x == destination_square.x || source_square.y == destination_square.y) {
                return true;
            }
            return false;
        }
    };

    auto static getStructure(Rook& obj) {
        return std::tie(obj.position, obj.model_name, obj.type, obj.is_white);
    };

}
