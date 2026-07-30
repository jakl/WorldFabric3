#pragma once
#include "Piece.h"

namespace Chess {

    class King :
        public Piece
    {
    public:

        King(const glm::vec3& position, Piece::COLOR color)
            : Piece(position, color, std::string("king") + (color ? "_white" : "_black")) { };

        King() = default;


        //This needs to be in every WorldObject to deduce types for serialziation templates from polymorphism
        // Just change the template parameter to match your class
        int getTypeId(Registry* r) const {
            return r->getIdForType<King>();
        }

        bool isValidMove(const glm::vec3& source_square, const glm::vec3& destination_square) const override {
            if (!Piece::isValidMove(source_square, destination_square)) { return false; }
            return fabs(source_square.x - destination_square.x) < 2 && fabs(source_square.z - destination_square.z) < 2;
        }
    };

    auto static getStructure(King& obj) {
        return std::tie(obj.position, obj.model_name, obj.color);
    };

}
