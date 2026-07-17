#include "ChessApp.h"
#include "ParticlePlugin.h"
#include "ScenePlugin.h"
#include "FlagSet.h"
#include <iostream>
#include <filesystem>


namespace Chess {
	ChessApp::ChessApp() {}

	void ChessApp::startSteamHosting() {
		SteamworksPlugin* steam = getTool<SteamworksPlugin>();
		WorldPlugin* world = getTool<WorldPlugin>();
		SteamworksPlugin::SteamServerInfo info;

		steam->setSteamEventReceiver(this);

		info.name = "Chess Server";
		info.map = "The Chess";
		info.max_players = 64;
		info.game_mode = "Classic";
		info.product_name = "Chess";
		info.product_description = "Be a piece. Hit a board.";
		info.game_directory = "Chess";
		info.version = "1";
		std::shared_ptr<Socket> steam_socket = steam->hostPrivateLobby(info);

		world->host(steam_socket, info.version);
	}


	// This is the client
	void ChessApp::onSteamGameExternalJoin(std::shared_ptr<SteamworksPlugin::SteamSocket> socket, const SteamworksPlugin::SteamServerInfo& server_info) {
		WorldPlugin* world = getTool<WorldPlugin>();
		SteamworksPlugin* steam = getTool<SteamworksPlugin>();

		PieceView::traceables.clear();
		cur_held_piece_id = -1;
		cur_held_piece.reset();

		std::shared_ptr<Socket> steam_socket = socket; // casting by creation avoids a warning
		world->disconnect();
		world->connect(steam_socket, "1");

		printf("Steam join of user : %s <of id %I64u >\n", steam->getLocalName().c_str(), steam->getLocalSteamID());
	}


	std::vector<std::string> ChessApp::listFileStems(const std::string& path) {
		std::vector<std::string> files_list;

		try {
			for (const auto& entry : std::filesystem::directory_iterator(path)) {
				if (std::filesystem::is_regular_file(entry.status())) {
					files_list.push_back(entry.path().stem().string());
				}
			}
		}
		catch (const std::filesystem::filesystem_error& e) {
			std::cerr << "Error: " << e.what() << std::endl;
		}

		return files_list;
	}

	void ChessApp::registerClassesAndMethods() {
		WorldPlugin* world = getTool<WorldPlugin>();

		world->registerClass<Piece, PieceView>("Piece");
		world->registerMethod(&Piece::setPosition, "setPosition");
		world->registerClass<Board, BoardView>("Board");
		world->registerMethod(&Board::init, "init");
		world->registerMethod(&Board::createBlackGlove, "createBlackGlove");
		world->registerClass<Glove, GloveView>("Glove");
		world->registerMethod(&Glove::setPosition, "setPosition");
	}

	void ChessApp::createWorldAndObjects() {
		WorldPlugin* world = getTool<WorldPlugin>();

		world->createWorld("chess", 10000, .0001f, 30);
		world->setTimeSpeed("chess", 1);

		//Make an instance of the board, which creates all the pieces
		std::shared_ptr<Board> board = std::shared_ptr<Board>(new Board(glm::vec3(0, 0, 0), "board"));
		int64_t board_id = world->create("chess", board);
		world->queue("chess", board_id, &Board::init);
	}

	void ChessApp::setupParticles() {
		ParticlePlugin* particles = getTool<ParticlePlugin>();
		mouse_particle_id = particles->createParticle(0);
	}

	void ChessApp::setupScene() {
		ScenePlugin* scene = getTool<ScenePlugin>();

		//Load all the game's blender models
		for (const auto& file_str : listFileStems("./assets/chess/glb/")) {
			scene->createModelSet(file_str, "./assets/chess/glb/" + file_str + ".glb");
		};

		// Set up a light for the scene
		ScenePlugin::LightComponent lc;
		glm::vec3 light_position = glm::vec3(4, 10, -10);
		lc.light_color = glm::vec4(0.9f, 0.9f, 0.9f, 1);
		light_effect_id = scene->createLight<ScenePlugin::ScreenPushConstants, ScenePlugin::LightComponent>(light_position, light_look_at, glm::vec3(0, 1, 0), 0.7f, 20, 1024, 0, lc);
		scene->destroyLight(light_effect_id - 1); // delete default light
	}

	// Called when switching into this state before the first time run is called
	void ChessApp::enter(std::shared_ptr<MachineState> from) {
		startSteamHosting();
		setupScene();
		registerClassesAndMethods();
		createWorldAndObjects();
		setupParticles();
	}

	//Called every frame while the state is active
	void ChessApp::run() {
		VulkanPlugin* window = getTool<VulkanPlugin>();
		WorldPlugin* world = getTool<WorldPlugin>();

		std::shared_ptr<const Board> board = world->observeNearest<Board>("chess");

		// Wait for game to load
		if (!board) {
			return;
		}

		auto boardView = world->getView<BoardView>("chess", board->id);

		// get the 3D ray from the mouse position on the screen
		glm::vec3 ray_origin = window->window_target->camera_position;
		glm::vec3 ray_direction = window->getMouseRay();
		int64_t closest_piece_id = -1;
		float mouse_ray_t = FLT_MAX;

		// Find which piece is under the mouse
		for (auto& [scene_id, obj_id] : PieceView::traceables) {
			std::shared_ptr<PieceView> piece_view = world->getView<PieceView>("chess", obj_id);
			float cur_t = raytrace(ray_origin, ray_direction, piece_view->scene_id, piece_view->pose);
			if (cur_t > 0 && cur_t < mouse_ray_t) {
				closest_piece_id = obj_id;
				mouse_ray_t = cur_t;
			}
		}

		// Left click holds a piece
		if (window->mouseDown(1) && !mouse_down_left) {
			if (cur_held_piece) {
				cur_held_piece_id = -1;
				cur_held_piece.reset();
			} else {
				cur_held_piece_id = closest_piece_id;
				cur_held_piece = world->observe<Piece>("chess", cur_held_piece_id);
			}
		}
		mouse_down_left = window->mouseDown(1);

		float board_t = raytrace(ray_origin, ray_direction, boardView->scene_id, boardView->pose);
		if (board_t > 0) {
			closest_piece_id = -1;

			glm::vec3 mouse_on_board_pos = ray_origin + ray_direction * board_t;

			if (world->amHosting()) {
				world->queue("chess", board->glove_white_id, &Glove::setPosition, mouse_on_board_pos);
			}
			else {
				if (board->glove_black_id == -1) {
					world->queue("chess", board->id, &Board::createBlackGlove);
				}
				else {
					world->queue("chess", board->glove_black_id, &Glove::setPosition, mouse_on_board_pos);
				}
			}

			if (cur_held_piece) {
				glm::vec3 mouse_piece_pos = mouse_on_board_pos;

				GlowUpHeldPiece();

				// Place pieces in the middle of squares
				mouse_piece_pos.x = std::round(mouse_piece_pos.x + .5f) - .5f;
				mouse_piece_pos.z = std::round(mouse_piece_pos.z + .5f) - .5f;

				// Only send the network event if the position is different
				if (cur_held_piece->position.x != mouse_piece_pos.x || cur_held_piece->position.z != mouse_piece_pos.z) {
					world->queue("chess", cur_held_piece_id, &Piece::setPosition, mouse_piece_pos);
					cur_held_piece = world->observe<Piece>("chess", cur_held_piece_id);
				}
			}
			else {

				ParticlePlugin* particles = getTool<ParticlePlugin>();
				particles->destroyParticle(mouse_particle_id);
			}
		}

		updateCamera();

		// Check if escape pressed to exit
		if (window->getLastKeyPress() == SDLK_ESCAPE) {
			getTool<FlagSet>()->setInt(AsyncPlugin::SHUTDOWN_FLAG, 1);
		}
	}

	void ChessApp::GlowUpHeldPiece() {
		ParticlePlugin* particles = getTool<ParticlePlugin>();

		glm::mat4 particle_pose = glm::mat4(1.0f);
		particles->setColor(mouse_particle_id, glm::vec4(1, 1, .5, .5)); // glow
		particle_pose = glm::translate(particle_pose, cur_held_piece->position);
		particle_pose = glm::scale(particle_pose, glm::vec3(.5f, .1f, .5f));
		particles->setPose(mouse_particle_id, particle_pose);
	}

	// Called when switching out of this state after the last time run is called
	void ChessApp::exit(std::shared_ptr<MachineState> to) {
		ScenePlugin* scene = getTool<ScenePlugin>();
		ParticlePlugin* particles = getTool<ParticlePlugin>();
		particles->destroyParticle(mouse_particle_id);
	}

	void ChessApp::updateCamera() {
		VulkanPlugin* window = getTool<VulkanPlugin>();
		ScenePlugin* scene = getTool<ScenePlugin>();
		if (window->mouseDown(3)) { // right mouse button
			if (!mouse_down_right) {
				mouse_down_position_right = window->getMousePosition();
				camera_down_thi = camera_thi;
				camera_down_theta = camera_theta;
			}
			glm::vec2 mouse_position = window->getMousePosition();
			mouse_down_right = true;
			camera_theta = camera_down_theta + camera_x_speed * (mouse_position.x - mouse_down_position_right.x);
			camera_thi = camera_down_thi + camera_y_speed * (mouse_position.y - mouse_down_position_right.y);
			camera_thi = fmax(fmin(camera_thi, 3.14159f * 0.5f), 0.0f);
			mouse_down_position_right = window->getMousePosition();
			camera_down_thi = camera_thi;
			camera_down_theta = camera_theta;

		}
		else {
			mouse_down_right = false;
		}

		if (mouse_wheel_y_previous < window->getMouseWheelPosition().y) {
			zoom *= 0.95f;
		}
		else if (mouse_wheel_y_previous > window->getMouseWheelPosition().y) {
			zoom /= 0.95f;
		}

		if (zoom < 1.0f) {
			zoom = 1.0f;
		}
		mouse_wheel_y_previous = window->getMouseWheelPosition().y;

		glm::vec3 camera_position = glm::vec3(cosf(camera_theta) * cosf(camera_thi), sinf(camera_thi), sinf(camera_theta) * cosf(camera_thi)) * zoom;
		window->window_target->setCamera(camera_position, look_at, fov, glm::vec3(0, 1, 0));

		glm::vec3 light_position = glm::vec3(cosf(camera_theta + light_theta_off) * cosf(camera_thi), sinf(camera_thi), sinf(camera_theta + light_theta_off) * cosf(camera_thi)) * light_zoom;

		scene->moveLight<ScenePlugin::ScreenPushConstants, ScenePlugin::LightComponent>(light_effect_id, light_position, light_look_at, glm::vec3(0, 1, 0), light_fov, 30);
	}

	float ChessApp::raytrace(const glm::vec3& pos, const glm::vec3& ray, int scene_id, glm::mat4 pose) {
		ScenePlugin* scene = getTool<ScenePlugin>();
		std::shared_ptr<GLTF> model = scene->getModelController(scene_id);
		if (!model) {
			return -1;
		}

		// Move the mouse ray into the model's coordinates
		glm::mat4 scene_to_model_space = glm::inverse(pose);
		glm::vec3 ray_origin = scene_to_model_space * glm::vec4(pos, 1); // positions have 1 in slot 4 to include translation
		glm::vec3 ray_direction = scene_to_model_space * glm::vec4(ray, 0); // directions have 0 in slot 4 to not include translation

		return model->rayTrace(ray_origin, ray_direction);
	}
}