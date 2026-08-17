#pragma once
#include "Piece.h"

namespace Chess {

    class King :
        public Piece
    {
    public:

        King(const glm::vec3& position, const int64_t& board_id, const Piece::COLOR& color)
            : Piece(position, board_id, color, std::string("king") + (color ? "_white" : "_black")) { };

        King() = default;


        //This needs to be in every WorldObject to deduce types for serialziation templates from polymorphism
        // Just change the template parameter to match your class
        int getTypeId(Registry* r) const {
            return r->getIdForType<King>();
        }

        bool isValidMove(const glm::vec3& destination) const override {
            if (!Piece::isValidMove(destination)) return false;

            WorldPlugin* world = getTool<WorldPlugin>();
            bool trying_to_castle = fabs(position.x - destination.x) == 2;
            bool trying_to_castle_east = destination.x > 0;
            float rook_x = trying_to_castle_east ? 3.5f : -3.5f;
            glm::vec3 rook_pos = glm::vec3(rook_x, 0, position.z);
            int64_t rook_id = piece_at(rook_pos);
            auto rook = world->observe<Rook>("chess", rook_id);
            bool can_castle = trying_to_castle && !has_moved && !rook->has_moved && !!blocked_by(rook_pos);

            if (can_castle) world->queue("chess", rook_id, &Rook::castle);

            return can_castle || fabs(position.x - destination.x) == 1 && fabs(position.z - destination.z) == 1;
        }
    };

    auto static getStructure(King& obj) {
        return std::tie(obj.position, obj.model_name, obj.color, obj.board_id, obj.has_moved);
    };

}
