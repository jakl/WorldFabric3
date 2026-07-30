#include "Board.h"

#include "VulkanPlugin.h"
#include "PanelPlugin.h"
#include "AudioPlugin.h"
#include "ChessApp.h"
#include "ParticlePlugin.h"
#include <print>

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
			pieces_ids.emplace_back(create(std::shared_ptr<Pawn>(new Pawn(glm::vec3(i - 3.5, 0, -2.5), Piece::black, id)), time));
			pieces_ids.emplace_back(create(std::shared_ptr<Pawn>(new Pawn(glm::vec3(i - 3.5, 0, 2.5), Piece::white, id)), time));
		}
		for (int i = 0; i < 2; i++) {
			pieces_ids.emplace_back(create(std::shared_ptr<Knight>(new Knight(glm::vec3(i * 5 - 2.5, 0, -3.5), Piece::black, id)), time));
			pieces_ids.emplace_back(create(std::shared_ptr<Knight>(new Knight(glm::vec3(i * 5 - 2.5, 0, 3.5), Piece::white, id)), time));
			pieces_ids.emplace_back(create(std::shared_ptr<Rook>(new Rook(glm::vec3(i * 7 - 3.5, 0, -3.5), Piece::black, id)), time));
			pieces_ids.emplace_back(create(std::shared_ptr<Rook>(new Rook(glm::vec3(i * 7 - 3.5, 0, 3.5), Piece::white, id)), time));
			pieces_ids.emplace_back(create(std::shared_ptr<Bishop>(new Bishop(glm::vec3(i * 3 - 1.5, 0, -3.5), Piece::black, id)), time));
			pieces_ids.emplace_back(create(std::shared_ptr<Bishop>(new Bishop(glm::vec3(i * 3 - 1.5, 0, 3.5), Piece::white, id)), time));
		}
		pieces_ids.emplace_back(create(std::shared_ptr<King>(new King(glm::vec3(.5, 0, -3.5), Piece::black, id)), time));
		pieces_ids.emplace_back(create(std::shared_ptr<King>(new King(glm::vec3(.5, 0, 3.5), Piece::white, id)), time));
		pieces_ids.emplace_back(create(std::shared_ptr<Queen>(new Queen(glm::vec3(-.5, 0, -3.5), Piece::black, id)), time));
		pieces_ids.emplace_back(create(std::shared_ptr<Queen>(new Queen(glm::vec3(-.5, 0, 3.5), Piece::white, id)), time));
		 
		//This is this player's hand (the hosting aka first player)
		glove_white_id = create(std::shared_ptr<Glove>(new Glove(glm::vec3(.5, 0, 3.5), "glove")), time);
	}

	void Board::createBlackGlove() {
		if (glove_black_id == -1) {
			glove_black_id = create(std::shared_ptr<Glove>(new Glove(glm::vec3(.5, 0, 3.5), "glove", false)), time);
		}
	}

	void Board::takePiece(const glm::vec3& square) {
		for (int64_t each_piece_id : pieces_ids) {
			std::shared_ptr<const WorldObject> world_object = read(each_piece_id);
			if (!world_object) { continue; }

			std::shared_ptr<const Piece> each_piece = std::dynamic_pointer_cast<const Piece>(world_object);
			if (each_piece && square == each_piece->position) {
				queue(each_piece_id, time, &Piece::destroy);
			}
		}
	}

	void BoardView::created(std::shared_ptr<const Board>& observation) {
		ScenePlugin* scene = getTool<ScenePlugin>();
		ActionMap* action_map = getTool<ActionMap>();
		ParticlePlugin* particles = getTool<ParticlePlugin>();

		last_observation = observation;

		pose = glm::mat4(1.0f);
		pose = glm::translate(pose, observation->position);
		scene_id = scene->createInstance(observation->model_name, pose);

		std::shared_ptr<GLTF> model = scene->getModelController(observation->model_name);
		//Note: multiplying pose by AABB corners only works to prdouce another correct AABB here when pose contains only translation and scale
		std::shared_ptr<ActionTrigger> trigger = std::shared_ptr<ActionTrigger>(new ActionTrigger(0,pose * glm::vec4(model->min,1), pose * glm::vec4(model->max, 1),this)) ;
		trigger_id = action_map->addTrigger(trigger);

		particle_id = particles->createParticle(0) ;
	}

	//Update is called when an observation is made of an object that was also observed last frame on this same view
	void BoardView::updated(std::shared_ptr<const Board>& observation) {
		last_observation = observation ;
	}

	//Destroyed is called when an observation that was present in the last observation is no longer observed
	//This view will be deleted immediately after this call (it's destructor will be called after this)
	void BoardView::destroyed() {
		ScenePlugin* scene = getTool<ScenePlugin>();
		ActionMap* action_map = getTool<ActionMap>();
		ParticlePlugin* particles = getTool<ParticlePlugin>();

		scene->deleteInstance(scene_id);
		action_map->removeTrigger(trigger_id);
		particles->destroyParticle(particle_id);
	}

	void BoardView::receiveAction(std::shared_ptr<ChessMouseAction>& action, std::shared_ptr<ActionTrigger>& trigger){
		WorldPlugin* world = getTool<WorldPlugin>();
		float board_t = ChessApp::raytrace(action->origin, action->direction, scene_id, pose);
		glm::vec3 mouse_on_board_pos = action->origin + action->direction * board_t;
		mouse_on_board_pos.y = 0;

		if (world->amHosting()) {
			world->queue("chess", last_observation->glove_white_id, &Glove::setPosition, mouse_on_board_pos);
		}
		else {
			if (last_observation->glove_black_id == -1) {
				world->queue("chess", last_observation->id, &Board::createBlackGlove);
			}
			else {
				world->queue("chess", last_observation->glove_black_id, &Glove::setPosition, mouse_on_board_pos);
			}
		}

		if (action->held_piece != -1) {
			std::shared_ptr<const Piece> piece = world->observe<Piece>("chess", action->held_piece);
			if(piece){
				glowUpHeldPiece(piece);
			}
			if(action->clicked || !piece){
				// Place pieces in the middle of squares
				glm::vec3 move_piece_to_p = mouse_on_board_pos;
				move_piece_to_p.x = std::round(move_piece_to_p.x + .5f) - .5f;
				move_piece_to_p.z = std::round(move_piece_to_p.z + .5f) - .5f;

				// Only send the network event if the position is different
				if (piece && piece->position.x != move_piece_to_p.x || piece->position.z != move_piece_to_p.z) {
					std::shared_ptr<const Board> board = world->observeNearest<Board>("chess");
					if (piece->isValidMove(piece->position, move_piece_to_p)) {
						world->queue("chess", board->id, &Board::takePiece, move_piece_to_p);
						world->queue("chess", piece->id, &Piece::setPosition, move_piece_to_p);
					}
				}
				action->next_held_piece = -1 ; // drop piece
			}
		}
		else {
			ParticlePlugin* particles = getTool<ParticlePlugin>();
			particles->setPose(particle_id, glm::mat4(0));
		}
	}

	void BoardView::glowUpHeldPiece(std::shared_ptr<const Piece>& piece) {
		ParticlePlugin* particles = getTool<ParticlePlugin>();
		glm::mat4 particle_pose = glm::mat4(1.0f);
		particles->setColor(particle_id, glm::vec4(1, 1, .5, .5)); // glow
		particle_pose = glm::translate(particle_pose, piece->position);
		particle_pose = glm::scale(particle_pose, glm::vec3(.5f, .1f, .5f));
		particles->setPose(particle_id, particle_pose);
	}
}