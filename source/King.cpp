#pragma once
#include "King.h"

namespace Chess {

std::list<int64_t> King::rookThreats() const {
    int64_t id;
    std::list<int64_t> ids;

    if (id = blockedBy(glm::vec3(position.x, 0, 4.5))) ids.emplace_back(id);
    if (id = blockedBy(glm::vec3(position.x, 0, -4.5))) ids.emplace_back(id);
    if (id = blockedBy(glm::vec3(4.5, 0, position.z))) ids.emplace_back(id);
    if (id = blockedBy(glm::vec3(-4.5, 0, position.z))) ids.emplace_back(id);

    return ids;
}

std::list<int64_t> King::bishopThreats() const {
    int64_t id;
    std::list<int64_t> ids;

    if (id = blockedBy(glm::vec3(position.x + 8, 0, position.z + 8))) ids.emplace_back(id);
    if (id = blockedBy(glm::vec3(position.x + 8, 0, position.z - 8))) ids.emplace_back(id);
    if (id = blockedBy(glm::vec3(position.x - 8, 0, position.z - 8))) ids.emplace_back(id);
    if (id = blockedBy(glm::vec3(position.x - 8, 0, position.z + 8))) ids.emplace_back(id);

    return ids;
}

std::list<int64_t> King::knightThreats() const {
    WorldPlugin* world = getTool<WorldPlugin>();
    auto board = world->observeNearest<Board>("chess");
    auto pieces = board->board_of_pieces;
    std::list<int64_t> ids;

    // TODO: Is there a "looped" way to do this more concisely?
    if (glm::vec3 p = glm::vec3(position.x - 1, 0, position.z - 2); pieces.contains(p)) ids.emplace_back(pieces.at(p));
    if (glm::vec3 p = glm::vec3(position.x + 1, 0, position.z - 2); pieces.contains(p)) ids.emplace_back(pieces.at(p));
    if (glm::vec3 p = glm::vec3(position.x - 1, 0, position.z + 2); pieces.contains(p)) ids.emplace_back(pieces.at(p));
    if (glm::vec3 p = glm::vec3(position.x + 1, 0, position.z + 2); pieces.contains(p)) ids.emplace_back(pieces.at(p));

    if (glm::vec3 p = glm::vec3(position.x - 2, 0, position.z - 1); pieces.contains(p)) ids.emplace_back(pieces.at(p));
    if (glm::vec3 p = glm::vec3(position.x + 2, 0, position.z - 1); pieces.contains(p)) ids.emplace_back(pieces.at(p));
    if (glm::vec3 p = glm::vec3(position.x - 2, 0, position.z + 1); pieces.contains(p)) ids.emplace_back(pieces.at(p));
    if (glm::vec3 p = glm::vec3(position.x + 2, 0, position.z + 1); pieces.contains(p)) ids.emplace_back(pieces.at(p));

    return ids;
}

bool King::inCheck() const {
    WorldPlugin* world = getTool<WorldPlugin>();
    for (const auto& id : rookThreats()) {
        auto piece = world->observe<Piece>("chess", id);
        if (piece->moves_like_rook) return true;

        if (fabs(piece->position.x - position.x) <= 1 && fabs(piece->position.z - position.z) <= 1) {
            auto king = world->observe<King>("chess", id);
            if (king) return true;
        }
    }
    for (const auto& id : bishopThreats()) {
        auto piece = world->observe<Piece>("chess", id);
        if (piece->moves_like_bishop) return true;

        bool pawn_threat = !!color ? piece->position.z > position.z : piece->position.z < position.z;
        if (fabs(piece->position.x - position.x) == 1 && fabs(piece->position.z - position.z) == 1 && pawn_threat) {
            auto pawn = world->observe<Pawn>("chess", id);
            if (pawn) return true;
        }

        if (fabs(piece->position.x - position.x) <= 1 && fabs(piece->position.z - position.z) <= 1) {
            auto king = world->observe<King>("chess", id);
            if (king) return true;
        }
    }
    for (const auto& id : knightThreats()) {
        auto knight = world->observe<Knight>("chess", id);
        if (knight) return true;
    }
}

}