#include "Piece.h"

#include "VulkanPlugin.h"
#include "PanelPlugin.h"
#include "AudioPlugin.h"
#include "ChessApp.h"
#include <print>
#include <format>

namespace Chess {

void Piece::setPosition(const glm::vec3& p) {
	position = p;
}

bool Piece::isValidMove(const glm::vec3& dest_p) const {
	return fabs(dest_p.x) < 3.8 && fabs(dest_p.z) < 3.8; // Destination is on the board
}

void Piece::destroy() {
	destroyed = true;
}

std::vector<glm::vec3> Piece::squaresBetween(const glm::vec3& from_p, const glm::vec3& to_p) const {
	std::vector<glm::vec3> squares;
	float from_x = from_p.x;
	float from_z = from_p.z;

	while (true) {
		if (from_x < to_p.x) { from_x++; }
		if (from_x > to_p.x) { from_x--; }
		if (from_z < to_p.z) { from_z++; }
		if (from_z > to_p.z) { from_z--; }

		if (from_x == to_p.x && from_z == to_p.z) { return squares; }

		squares.emplace_back(glm::vec3(from_x, 0, from_z));
	}
}

// This is mandated by the engine
void PieceView::created(std::shared_ptr<const Piece>& observation) {
	ScenePlugin* scene = getTool<ScenePlugin>();
	ActionMap* action_map = getTool<ActionMap>();

	last_observation = observation;

	pose = glm::mat4(1.0f);
	pose = glm::translate(pose, observation->position);
	scene_id = scene->createInstance(observation->model_name, pose);
	//traceables[scene_id] = observation.id;
	
	std::shared_ptr<GLTF> model = scene->getModelController(observation->model_name);
	//Note: multiplying pose by AABB corners only works to prdouce another correct AABB here when pose contains only translation and scale
	std::shared_ptr<ActionTrigger> trigger = std::shared_ptr<ActionTrigger>(new ActionTrigger(0, pose * glm::vec4(model->min, 1), pose * glm::vec4(model->max, 1), this));
	trigger_id = action_map->addTrigger(trigger);
}

// This is mandated by the engine
//Update is called when an observation is made of an object that was also observed last frame on this same view
void PieceView::updated(std::shared_ptr<const Piece>& observation) {
	ScenePlugin* scene = getTool<ScenePlugin>();
	ActionMap* action_map = getTool<ActionMap>();

	last_observation = observation;

	pose = glm::mat4(1.0f);
	pose = glm::translate(pose, observation->position);
	scene->setPose(scene_id, pose);

	std::shared_ptr<GLTF> model = scene->getModelController(observation->model_name);
	action_map->moveTrigger(trigger_id, pose * glm::vec4(model->min, 1), pose * glm::vec4(model->max, 1));

}

// This is mandated by the engine
//Destroyed is called when an observation that was present in the last observation is no longer observed
//This view will be deleted immediately after this call (it's destructor will be called after this)
void PieceView::destroyed() {
	ScenePlugin* scene = getTool<ScenePlugin>();
	ActionMap* action_map = getTool<ActionMap>();
	scene->deleteInstance(scene_id);
	action_map->removeTrigger(trigger_id);
}

void PieceView::receiveAction(std::shared_ptr<ChessMouseAction>& action, std::shared_ptr<ActionTrigger>& trigger) {
	if (action->next_held_piece == -1 && action->clicked) { // piece not currently held and click is pressed
		float t = ChessApp::raytrace(action->origin, action->direction,scene_id,pose) ;
		if(t > 0){// Only act if the actual model was hit and not just he bounding box
			// recieveAction is called in ray order, so setting this will stop others from getting into the first if
			action->next_held_piece = last_observation->id ;
		}
	}
}

}