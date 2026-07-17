#include "Board.h"

#include "VulkanPlugin.h"
#include "PanelPlugin.h"
#include "AudioPlugin.h"
#include "glm/glm.hpp"


namespace Chess {

	Board::Board(const glm::vec3& p, const std::string& model_name_set) {
		position = p;
		model_name = model_name_set;
	}

	void Board::destroy() {
		destroyed = true;
	}

	void Board::print() const {
		printf("Board at %f, %f, %f\n", position.x, position.y, position.z);
	}

	void Board::init() {
		WorldPlugin* world = getTool<WorldPlugin>();

		// Initialize all the chess pieces
		for (int i = 0; i < 8; i++) {
			create(std::shared_ptr<Piece>(new Piece(glm::vec3(i - 3.5, 0, -2.5), "pawn_black")), time);
			create(std::shared_ptr<Piece>(new Piece(glm::vec3(i - 3.5, 0, 2.5), "pawn_white")), time);
		}
		for (int i = 0; i < 2; i++) {
			create(std::shared_ptr<Piece>(new Piece(glm::vec3(i * 5 - 2.5, 0, -3.5), "knight_black")), time);
			create(std::shared_ptr<Piece>(new Piece(glm::vec3(i * 5 - 2.5, 0, 3.5), "knight_white")), time);
			create(std::shared_ptr<Piece>(new Piece(glm::vec3(i * 7 - 3.5, 0, -3.5), "rook_black")), time);
			create(std::shared_ptr<Piece>(new Piece(glm::vec3(i * 7 - 3.5, 0, 3.5), "rook_white")), time);
			create(std::shared_ptr<Piece>(new Piece(glm::vec3(i * 3 - 1.5, 0, -3.5), "bishop_black")), time);
			create(std::shared_ptr<Piece>(new Piece(glm::vec3(i * 3 - 1.5, 0, 3.5), "bishop_white")), time);
		}

		//Make instances of the black royalty
		create(std::shared_ptr<Piece>(new Piece(glm::vec3(.5, 0, -3.5), "king_black")), time);
		create(std::shared_ptr<Piece>(new Piece(glm::vec3(-.5, 0, -3.5), "queen_black")), time);
		//Make instances of the white royalty
		create(std::shared_ptr<Piece>(new Piece(glm::vec3(.5, 0, 3.5), "king_white")), time);
		create(std::shared_ptr<Piece>(new Piece(glm::vec3(-.5, 0, 3.5), "queen_white")), time);
		// This is this player's hand (the hosting aka first player)
		glove_white_id = create(std::shared_ptr<Glove>(new Glove(glm::vec3(.5, 0, 3.5), "glove")), time);
	}

	void Board::createBlackGlove() {
		if (glove_black_id == -1) {
			glove_black_id = create(std::shared_ptr<Glove>(new Glove(glm::vec3(.5, 0, 3.5), "glove", false)), time);
		}
	}

	void BoardView::created(const Board& observation) {
		ScenePlugin* scene = getTool<ScenePlugin>();

		id = observation.id;
		pose = glm::mat4(1.0f);
		pose = glm::translate(pose, observation.position);
		scene_id = scene->createInstance(observation.model_name, pose);
	}

	//Update is called when an observation is made of an object that was also observed last frame on this same view
	void BoardView::updated(const Board& observation) {

	}

	//Destroyed is called when an observation that was present in the last observation is no longer observed
	//This view will be deleted immediately after this call (it's destructor will be called after this)
	void BoardView::destroyed() {
		//printf("Board view destroyed: %ld\n", (long)id);
		ScenePlugin* scene = getTool<ScenePlugin>();
		scene->deleteInstance(scene_id);
	}

}