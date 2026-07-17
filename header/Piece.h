#ifndef _Piece_Piece_H_
#define _Piece_Piece_H_ 1


#include "WorldPlugin.h"
#include "ScenePlugin.h"
#include "PanelPlugin.h"
#include "glm/glm.hpp"
#include <stdio.h>
#include <cstdlib>

#include <string>
#include <map>
#include <set>

namespace Chess {

class Piece : public WorldObject {
public:

	std::string model_name = "pawn_white";
	enum TYPE { pawn, rook, knight, bishop, king, queen };
	TYPE type = TYPE::pawn;
	bool is_white = true;

	Piece(const glm::vec3& p, const std::string& model_name_set);

	//Functions to be used as events must be void return and only const& parameters
	// Also they're not allowed to read or write any data outside the object except through timeline functions
	void setPosition(const glm::vec3& p);
	void destroy();

	//Functions used on observables or on read objects need to be const
	void print() const override;

	Piece() {} // WorldObject's need a default constructor to make an object to deserialize into
	virtual ~Piece() = default; // Force to be polymorphic just in case

	//This needs to be in every WorldObject to deduce types for serialziation templates from polymorphism
	// Just change the template parameter to match your class
	int getTypeId(Registry* r) const {
		return r->getIdForType<Piece>();
	}
};


class PieceView : public ObjectView<Piece> {
public:
	int64_t id;
	int scene_id = -1;
	glm::mat4 pose;
	static inline std::map<int, int64_t> traceables;

	//created is called when an objectis observed that ws no observed last time view was called on the world
	void created(const Piece& observation) override;

	//Update is called when an observation is made of an object that was also observed last frame on this same view
	void updated(const Piece& observation) override;

	//Destroyed is called when an observation that was present in the last observation is no longer observed
	//This view will be deleted immediately after this call (it's destructor will be called after this)
	void destroyed() override;

	//Computes the scene pose of a Piece
	glm::mat4 computePose(const Piece& Piece);
};

auto static getStructure(Piece& obj) {
	return std::tie(obj.position, obj.model_name, obj.type, obj.is_white);
};


} // end Piece name space

#endif // #ifndef _Piece_Piece_H_