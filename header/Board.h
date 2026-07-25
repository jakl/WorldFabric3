#ifndef _Board_Board_H_
#define _Board_Board_H_ 1


#include "WorldPlugin.h"
#include "ScenePlugin.h"
#include "PanelPlugin.h"
#include "ChessMouseAction.h"
#include <stdio.h>
#include <cstdlib>
#include <Piece.h>

#include <string>
#include <map>
#include <set>
#include <Glove.h>
#include <Rook.h>
#include <Pawn.h>
#include <King.h>
#include <Queen.h>
#include <Bishop.h>
#include <Knight.h>

namespace Chess {


	class Board : public WorldObject {
	public:

		std::string model_name;
		int64_t glove_white_id = -1;
		int64_t glove_black_id = -1;

		Board(const glm::vec3& p, const std::string& model_name_set);
		void init();

		std::vector<std::vector<int64_t>> grid;
		void destroy();

		//Functions used on observables or on read objects need to be const
		void print() const override;

		Board() {} // WorldObject's need a default constructor to make an object to deserialize into
		virtual ~Board() = default; // Force to be polymorphic just in case

		void createBlackGlove();

		//This needs to be in every WorldObject to deduce types for serialziation templates from polymorphism
		// Just change the template parameter to match your class
		int getTypeId(Registry* r) const {
			return r->getIdForType<Board>();
		}

	};


	class BoardView : public ObjectView<Board>, public virtual ActionReceiver<ChessMouseAction> {
	public:
		Board last_observation;
		int scene_id = -1;
		int trigger_id = -1;
		int particle_id = -1 ;
		glm::mat4 pose;
		

		//created is called when an objectis observed that ws no observed last time view was called on the world
		void created(const Board& observation) override;

		//Update is called when an observation is made of an object that was also observed last frame on this same view
		void updated(const Board& observation) override;

		//Destroyed is called when an observation that was present in the last observation is no longer observed
		//This view will be deleted immediately after this call (it's destructor will be called after this)
		void destroyed() override;

		void receiveAction(std::shared_ptr<ChessMouseAction>& action, std::shared_ptr<ActionTrigger>& trigger) override;

		void glowUpHeldPiece(std::shared_ptr<const Piece>& piece) ;
	};

	auto static getStructure(Board& obj) {
		return std::tie(obj.position, obj.model_name, obj.glove_black_id, obj.glove_white_id);
	}


} // end Board name space

#endif // #ifndef _Board_Board_H_