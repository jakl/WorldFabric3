#pragma once
#include "Piece.h"

namespace Chess {

    class Pawn :
        public Piece
    {
    public:

        Pawn(const glm::vec3& position, Piece::COLOR color, int64_t board_id)
            : Piece(position, color, std::string("pawn") + (color ? "_white" : "_black"), board_id) {};

        Pawn() = default;


        //This needs to be in every WorldObject to deduce types for serialziation templates from polymorphism
        // Just change the template parameter to match your class
        int getTypeId(Registry* r) const {
            return r->getIdForType<Pawn>();
        }

        bool isValidMove(const glm::vec3& source_square, const glm::vec3& destination_square) const override {
            if (!Piece::isValidMove(source_square, destination_square)) { return false; }
            bool moved_towards_black = source_square.z - destination_square.z == 1;
            bool moved_one_square = fabs(source_square.z - destination_square.z) == 1;
            bool is_white = !!color;
            return is_white == moved_towards_black && source_square.x == destination_square.x && moved_one_square;
        }
    };

    auto static getStructure(Pawn& obj) {
        return std::tie(obj.position, obj.model_name, obj.color);
    };

}
