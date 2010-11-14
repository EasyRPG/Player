/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
//
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

#ifndef _LDB_READER_H_
#define _LDB_READER_H_

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <string>
#include <iostream>
#include "main_data.h"
#include "reader.h"

////////////////////////////////////////////////////////////
/// LDB Reader namespace
////////////////////////////////////////////////////////////
namespace LDB_Reader {
	bool Load(const std::string &filename);
	void LoadChunks(Reader& stream);

	RPG::Actor ReadActor(Reader& stream);
	RPG::Learning ReadLearning(Reader& stream);
	RPG::Skill ReadSkill(Reader& stream);
	RPG::Item ReadItem(Reader& stream);
	RPG::Enemy ReadEnemy(Reader& stream);
	RPG::EnemyAction ReadEnemyAction(Reader& stream);
	RPG::Troop ReadTroop(Reader& stream);
	RPG::TroopMember ReadTroopMember(Reader& stream);
	RPG::TroopPage ReadTroopPage(Reader& stream);
	RPG::TroopPageCondition ReadTroopPageCondition(Reader& stream);
	RPG::Attribute ReadAttribute(Reader& stream);
	RPG::State ReadState(Reader& stream);
	RPG::Terrain ReadTerrain(Reader& stream);
	RPG::Chipset ReadChipset(Reader& stream);
	RPG::Animation ReadAnimation(Reader& stream);
	RPG::AnimationTiming ReadAnimationTiming(Reader& stream);
	RPG::AnimationFrame ReadAnimationFrame(Reader& stream);
	RPG::AnimationCellData ReadAnimationCellData(Reader& stream);
	RPG::Terms ReadTerms(Reader& stream);
	RPG::System ReadSystem(Reader& stream);
	RPG::CommonEvent ReadCommonEvent(Reader& stream);
	RPG::Sound ReadSound(Reader& stream);
	RPG::Music ReadMusic(Reader& stream);
	std::vector<std::string> ReadVariables(Reader& stream);
	std::vector<std::string> ReadSwitches(Reader& stream);
	RPG::TestBattler ReadTestBattler(Reader& stream);

	// 2003
	RPG::BattleCommand ReadBattleCommand(Reader& stream);
	RPG::Class ReadClass(Reader& stream);
	RPG::BattlerAnimation ReadBattlerAnimation(Reader& stream);
	RPG::BattlerAnimationExtension ReadBattlerAnimationExtension(Reader& stream);
}

#endif
