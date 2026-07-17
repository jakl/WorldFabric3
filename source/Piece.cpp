#include "Piece.h"

#include "VulkanPlugin.h"
#include "PanelPlugin.h"
#include "AudioPlugin.h"
#include "glm/glm.hpp"


namespace Chess {

Piece::Piece(const glm::vec3& p, const std::string& model_name_set) {
	position = p;
	model_name = model_name_set;

	std::istringstream stream(model_name_set);
	std::string delimited;
	while (std::getline(stream, delimited, '_')) {
		if ("white"  == delimited) is_white = true;
		if ("black"  == delimited) is_white = false;
		if ("pawn"   == delimited) type = TYPE::pawn;
		if ("rook"   == delimited) type = TYPE::rook;
		if ("knight" == delimited) type = TYPE::knight;
		if ("bishop" == delimited) type = TYPE::bishop;
		if ("king"   == delimited) type = TYPE::king;
		if ("queen"  == delimited) type = TYPE::queen;
	}
}

void Piece::setPosition(const glm::vec3& p) {
	if (fabs(p.x) < 3.8 && fabs(p.z) < 3.8) {
		position = p;
	}
}

void Piece::destroy() {
	destroyed = true;
}

void Piece::print() const {
	printf("Piece at %f, %f, %f\n", position.x, position.y, position.z);
}

// This is mandated by the engine
void PieceView::created(const Piece& observation) {
	pose = glm::mat4(1.0f);
	pose = glm::translate(pose, observation.position);
	ScenePlugin* scene = getTool<ScenePlugin>();
	scene_id = scene->createInstance(observation.model_name, pose);
	traceables[scene_id] = observation.id;
}

// This is mandated by the engine
//Update is called when an observation is made of an object that was also observed last frame on this same view
void PieceView::updated(const Piece& observation) {
	ScenePlugin* scene = getTool<ScenePlugin>();
	pose = glm::mat4(1.0f);
	pose = glm::translate(pose, observation.position);
	scene->setPose(scene_id, pose);
}

// This is mandated by the engine
//Destroyed is called when an observation that was present in the last observation is no longer observed
//This view will be deleted immediately after this call (it's destructor will be called after this)
void PieceView::destroyed() {
	//printf("Piece view destroyed: %ld\n", (long)id);
	ScenePlugin* scene = getTool<ScenePlugin>();
	scene->deleteInstance(scene_id);
}

}