#ifndef _Piece_Piece_H_
#define _Piece_Piece_H_ 1


#include "WorldPlugin.h"
#include "ScenePlugin.h"
#include "PanelPlugin.h"
#include "ChessMouseAction.h"
#include <stdio.h>
#include <cstdlib>

#include <string>
#include <map>
#include <set>
#include <print>

namespace Chess {

class Piece : public WorldObject {
public:

	enum COLOR { black, white }; // never change the order of these lol!
	std::string model_name;
	int64_t board_id;
	COLOR color;

	Piece(const glm::vec3& position, COLOR color, const std::string& model_name)
		: WorldObject(position), color(color), model_name(model_name) {};

	//Functions to be used as events must be void return and only const& parameters
	// Also they're not allowed to read or write any data outside the object except through timeline functions
	void setPosition(const glm::vec3& p);
	void destroy();

	//Functions used on observables or on read objects need to be const
	void print() const override;

	Piece() = default; // WorldObject's need a default constructor to make an object to deserialize into
	virtual ~Piece() = default; // Force to be polymorphic just in case

	virtual bool isValidMove(const glm::vec3& source_square, const glm::vec3& destination_square) const = 0;
};


class PieceView : public ObjectView<Piece>, public virtual ActionReceiver<ChessMouseAction> {
public:
	int scene_id = -1;
	int trigger_id = -1;
	glm::mat4 pose;
	std::shared_ptr<const Piece> last_observation;

	//created is called when an objectis observed that ws no observed last time view was called on the world
	void created(std::shared_ptr<const Piece>& observation) override;

	//Update is called when an observation is made of an object that was also observed last frame on this same view
	void updated(std::shared_ptr<const Piece>& observation) override;

	//Destroyed is called when an observation that was present in the last observation is no longer observed
	//This view will be deleted immediately after this call (it's destructor will be called after this)
	void destroyed() override;

	void receiveAction(std::shared_ptr<ChessMouseAction>& action, std::shared_ptr<ActionTrigger>& trigger) override;
};


} // end Piece name space

#endif // #ifndef _Piece_Piece_H_