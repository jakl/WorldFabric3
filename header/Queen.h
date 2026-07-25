#pragma once
#include "Piece.h"

namespace Chess {

    class Queen :
        public Piece
    {
    public:

        Queen(const glm::vec3& position, const std::string& model_name, bool is_white)
            : Piece(position, model_name, TYPE::queen, is_white) {};

        Queen() = default;


        //This needs to be in every WorldObject to deduce types for serialziation templates from polymorphism
        // Just change the template parameter to match your class
        int getTypeId(Registry* r) const {
            return r->getIdForType<Queen>();
        }

        bool isValidMove(const glm::vec2& source_square, const glm::vec2& destination_square) const override {
            if (source_square.x == destination_square.x || source_square.y == destination_square.y) {
                return true;
            }
            return false;
        }
    };

    auto static getStructure(Queen& obj) {
        return std::tie(obj.position, obj.model_name, obj.type, obj.is_white);
    };

}
