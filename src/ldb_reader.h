//////////////////////////////////////////////////////////////////////////////////
/// This file is part of EasyRPG Player.
/// 
/// EasyRPG Player is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
/// 
/// EasyRPG Player is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
/// 
/// You should have received a copy of the GNU General Public License
/// along with EasyRPG Player.  If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////////////////

#ifndef _LDB_READER_H_
#define _LDB_READER_H_

////////////////////////////////////////////////////////////
/// Headers
////////////////////////////////////////////////////////////
#include <string>
#include <iostream>
#include "main_data.h"

////////////////////////////////////////////////////////////
/// LDB Reader namespace
////////////////////////////////////////////////////////////
namespace LDB_Reader {
    void Load(const std::string &filename);
    void LoadChunks(FILE* stream);
    RPG::Actor ReadActor(FILE* stream);
    RPG::Learning ReadLearning(FILE* stream);
    RPG::Skill ReadSkill(FILE* stream);
    RPG::Item ReadItem(FILE* stream);
    RPG::Enemy ReadEnemy(FILE* stream);
    RPG::EnemyAction ReadEnemyAction(FILE* stream);
    RPG::Troop ReadTroop(FILE* stream);
    RPG::TroopMember ReadTroopMember(FILE* stream);
    RPG::TroopPage ReadTroopPage(FILE* stream);
    RPG::TroopPageCondition ReadTroopPageCondition(FILE* stream);
    RPG::Attribute ReadAttribute(FILE* stream);
    RPG::State ReadState(FILE* stream);
    RPG::Terrain ReadTerrain(FILE* stream);
    RPG::Chipset ReadChipset(FILE* stream);
    RPG::Animation ReadAnimation(FILE* stream);
    RPG::AnimationTiming ReadAnimationTiming(FILE* stream);
    RPG::AnimationFrame ReadAnimationFrame(FILE* stream);
    RPG::AnimationCellData ReadAnimationCellData(FILE* stream);
    RPG::Terms ReadTerms(FILE* stream);
    RPG::System ReadSystem(FILE* stream);
    RPG::CommonEvent ReadCommonEvent(FILE* stream);
    RPG::Sound ReadSound(FILE* stream);
    RPG::Music ReadMusic(FILE* stream);
    std::vector<std::string> ReadVariables(FILE* stream);
    std::vector<std::string> ReadSwitches(FILE* stream);
    RPG::TestBattler ReadTestBattler(FILE* stream);

    // 2003
    RPG::BattleCommand ReadBattleCommand(FILE* stream);
    RPG::Class ReadClass(FILE* stream);
    RPG::BattlerAnimation ReadBattlerAnimation(FILE* stream);
    RPG::BattlerAnimationExtension ReadBattlerAnimationExtension(FILE* stream);
}

#endif
