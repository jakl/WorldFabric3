#pragma once
#include "King.h"

namespace Chess {

// Return all pieces that are first seen from the king's perspective on his file/row
std::list<int64_t> King::rookThreats(const glm::vec3& new_p) const {
    int64_t id;
    std::list<int64_t> ids;

    if (id = blockedBy(glm::vec3(new_p.x, 0, 4.5))) ids.emplace_back(id);
    if (id = blockedBy(glm::vec3(new_p.x, 0, -4.5))) ids.emplace_back(id);
    if (id = blockedBy(glm::vec3(4.5, 0, new_p.z))) ids.emplace_back(id);
    if (id = blockedBy(glm::vec3(-4.5, 0, new_p.z))) ids.emplace_back(id);

    return ids;
}

// Return all pieces that are first seen from the king's perspective on his diagonals
std::list<int64_t> King::bishopThreats(const glm::vec3& new_p) const {
    int64_t id;
    std::list<int64_t> ids;

    if (id = blockedBy(glm::vec3(new_p.x + 8, 0, new_p.z + 8))) ids.emplace_back(id);
    if (id = blockedBy(glm::vec3(new_p.x + 8, 0, new_p.z - 8))) ids.emplace_back(id);
    if (id = blockedBy(glm::vec3(new_p.x - 8, 0, new_p.z - 8))) ids.emplace_back(id);
    if (id = blockedBy(glm::vec3(new_p.x - 8, 0, new_p.z + 8))) ids.emplace_back(id);

    return ids;
}

// Return all pieces that are seen from the king's perspective if he was a knight
std::list<int64_t> King::knightThreats(const glm::vec3& new_p) const {
    WorldPlugin* world = getTool<WorldPlugin>();
    auto board = world->observeNearest<Board>("chess");
    auto pieces = board->board_of_pieces;
    std::list<int64_t> ids;

    // TODO: Is there a "looped" way to do this more concisely?
    if (glm::vec3 p = glm::vec3(new_p.x - 1, 0, new_p.z - 2); pieces.contains(p)) ids.emplace_back(pieces.at(p));
    if (glm::vec3 p = glm::vec3(new_p.x + 1, 0, new_p.z - 2); pieces.contains(p)) ids.emplace_back(pieces.at(p));
    if (glm::vec3 p = glm::vec3(new_p.x - 1, 0, new_p.z + 2); pieces.contains(p)) ids.emplace_back(pieces.at(p));
    if (glm::vec3 p = glm::vec3(new_p.x + 1, 0, new_p.z + 2); pieces.contains(p)) ids.emplace_back(pieces.at(p));

    if (glm::vec3 p = glm::vec3(new_p.x - 2, 0, new_p.z - 1); pieces.contains(p)) ids.emplace_back(pieces.at(p));
    if (glm::vec3 p = glm::vec3(new_p.x + 2, 0, new_p.z - 1); pieces.contains(p)) ids.emplace_back(pieces.at(p));
    if (glm::vec3 p = glm::vec3(new_p.x - 2, 0, new_p.z + 1); pieces.contains(p)) ids.emplace_back(pieces.at(p));
    if (glm::vec3 p = glm::vec3(new_p.x + 2, 0, new_p.z + 1); pieces.contains(p)) ids.emplace_back(pieces.at(p));

    return ids;
}

bool King::inRookLikeCheck(const glm::vec3& new_p) const {
    WorldPlugin* world = getTool<WorldPlugin>();

    for (const auto& id : rookThreats(new_p)) {
        auto piece = world->observe<Piece>("chess", id);
        if (piece->color == color) continue;

        if (piece->moves_like_rook) return true;

        // Check for adjacent enemy king among rook-like threats
        if (fabs(piece->position.x - new_p.x) <= 1 && fabs(piece->position.z - new_p.z) <= 1) {
            auto king = world->observe<King>("chess", id);
            if (king) return true;
        }
    }

    return false;
}

bool King::inBishopLikeCheck(const glm::vec3& new_p) const {
    WorldPlugin* world = getTool<WorldPlugin>();

    for (const auto& id : bishopThreats(new_p)) {
        auto piece = world->observe<Piece>("chess", id);
        if (piece->color == color) continue;

        if (piece->moves_like_bishop) return true;

        // Check for diagonally adjacent enemy pawn among bishop-like threats
        bool pawn_threat = !!color ? piece->position.z > new_p.z : piece->position.z < new_p.z;
        if (fabs(piece->position.x - new_p.x) == 1 && fabs(piece->position.z - new_p.z) == 1 && pawn_threat) {
            auto pawn = world->observe<Pawn>("chess", id);
            if (pawn) return true;
        }

        // Check for diagonally adjacent enemy king among bishop-like threats
        if (fabs(piece->position.x - new_p.x) <= 1 && fabs(piece->position.z - new_p.z) <= 1) {
            auto king = world->observe<King>("chess", id);
            if (king) return true;
        }
    }

    return false;
}

bool King::inKnightLikeCheck(const glm::vec3& new_p) const {
    WorldPlugin* world = getTool<WorldPlugin>();

    for (const auto& id : knightThreats(new_p)) {
        auto knight = world->observe<Knight>("chess", id);
        if (knight && knight->color != color) return true; 
    }

    return false;
}


bool King::inCheck(const glm::vec3& new_p) const {
    return inRookLikeCheck(new_p) || inBishopLikeCheck(new_p) || inKnightLikeCheck(new_p);
}

}