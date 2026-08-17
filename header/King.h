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
            if (!Piece::isValidMove(destination)){
				return false;
			}
			// normal move
			if((fabs(position.x - destination.x) <= 1.1f && fabs(position.z - destination.z) <= 1.1f)){
				return true ; 
			}
			//Move 2 in x and not in z
			bool trying_to_castle = fabs(fabs(destination.x-position.x) - 2.0f) < 0.01f && fabs(position.z - destination.z) < 0.01f;
			if(!trying_to_castle){
				return false;
			}
			
            WorldPlugin* world = getTool<WorldPlugin>();
			bool trying_to_castle_east = destination.x > 0;
            float rook_x = trying_to_castle_east ? 3.5f : -3.5f;
            glm::vec3 rook_pos = glm::vec3(rook_x, 0, position.z);
			int64_t rook_id = piece_at(rook_pos);
            auto rook = world->observe<Rook>("chess", rook_id);
			if(rook){
				bool can_castle = !has_moved && !rook->has_moved && !blocked_by(rook_pos);

				if (can_castle){ // TODO don't queue from isValidMove?
					world->queue("chess", rook_id, &Rook::castle);
					return true ;
				}
			}

            return false;
        }
    };

    auto static getStructure(King& obj) {
        return std::tie(obj.position, obj.model_name, obj.color, obj.board_id, obj.has_moved);
    };

}
