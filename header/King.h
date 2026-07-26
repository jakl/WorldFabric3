#pragma once
#include "Piece.h"

namespace Chess {

    class King :
        public Piece
    {
    public:

        King(const glm::vec3& position, bool is_white)
            : Piece(position, is_white, "king" + is_white ? "_white" : "_black") {};

        King() = default;


        //This needs to be in every WorldObject to deduce types for serialziation templates from polymorphism
        // Just change the template parameter to match your class
        int getTypeId(Registry* r) const {
            return r->getIdForType<King>();
        }

        bool isValidMove(const glm::vec2& source_square, const glm::vec2& destination_square) const override {
            if (fabs(source_square.x - destination_square.x) < 2 && fabs(source_square.y - destination_square.y) < 2) {
                return true;
            }
            return false;
        }
    };

    auto static getStructure(King& obj) {
        return std::tie(obj.position, obj.model_name, obj.type, obj.is_white);
    };

}
