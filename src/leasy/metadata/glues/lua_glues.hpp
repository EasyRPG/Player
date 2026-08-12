//
// Created by @wys on 08/08/2026.
//

#ifndef EASYRPG_PLAYER_LUA_GLUES_HPP
#define EASYRPG_PLAYER_LUA_GLUES_HPP

#include "leasy/metadata/Domain.hpp"
#include <filesystem>

namespace leasy::metadata::glues {
  void generateLuaGlue(const std::string&, const Domain&, const std::filesystem::path&);
}

#endif //EASYRPG_PLAYER_LUA_GLUES_HPP
