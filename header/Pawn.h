#pragma once
#include "Piece.h"

namespace Chess {

    class Pawn :
        public Piece
    {
    public:

        Pawn(const glm::vec3& position, const std::string& model_name, bool is_white)
            : Piece(position, model_name, TYPE::pawn, is_white) {
        };

        Pawn() = default;


        //This needs to be in every WorldObject to deduce types for serialziation templates from polymorphism
        // Just change the template parameter to match your class
        int getTypeId(Registry* r) const {
            return r->getIdForType<Pawn>();
        }

        bool isValidMove(const glm::vec2& source_square, const glm::vec2& destination_square) const override {
            if (fabs(source_square.y - destination_square.y) < 2 && source_square.x == destination_square.x) {
                return true;
            }
            return false;
        }
    };

    auto static getStructure(Pawn& obj) {
        return std::tie(obj.position, obj.model_name, obj.type, obj.is_white);
    };

}
