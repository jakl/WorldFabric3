#pragma once
#include "Piece.h"

namespace Chess {

    class King :
        public Piece
    {
    public:

        King(const glm::vec3& position, const std::string& model_name, bool is_white)
            : Piece(position, model_name, TYPE::king, is_white) {};

        King() = default;


        //This needs to be in every WorldObject to deduce types for serialziation templates from polymorphism
        // Just change the template parameter to match your class
        int getTypeId(Registry* r) const {
            return r->getIdForType<King>();
        }

        bool isValidMove(const glm::vec2& source_square, const glm::vec2& destination_square) const override {
            if (source_square.x == destination_square.x || source_square.y == destination_square.y) {
                return true;
            }
            return false;
        }
    };

    auto static getStructure(King& obj) {
        return std::tie(obj.position, obj.model_name, obj.type, obj.is_white);
    };

}
