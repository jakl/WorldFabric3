#pragma once
#include "Piece.h"

namespace Chess {

    class Bishop :
        public Piece
    {
    public:

        Bishop(const glm::vec3& position, bool is_white)
            : Piece(position, is_white, "bishop" + is_white ? "_white" : "_black") {};

        Bishop() = default;


        //This needs to be in every WorldObject to deduce types for serialziation templates from polymorphism
        // Just change the template parameter to match your class
        int getTypeId(Registry* r) const {
            return r->getIdForType<Bishop>();
        }

        bool isValidMove(const glm::vec2& source_square, const glm::vec2& destination_square) const override {
            if (fabs(source_square.x - destination_square.x) - fabs(source_square.y - destination_square.y) == 0) {
                return true;
            }
            return false;
        }
    };

    auto static getStructure(Bishop& obj) {
        return std::tie(obj.position, obj.model_name, obj.type, obj.is_white);
    };

}
