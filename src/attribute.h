/*
 * This file is part of EasyRPG Player.
 *
 * EasyRPG Player is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EP_ATTRIBUTE_H
#define EP_ATTRIBUTE_H

#include <lcf/rpg/fwd.h>
#include <lcf/dbbitarray.h>
#include <span.h>

class Game_Battler;

/** Namespace of functions dealing with attribute modifiers */
namespace Attribute {

/**
 * Gets the attribute damage multiplier/protection (A-E).
 *
 * @param attribute_id Attribute to test
 * @param rate Attribute rate to get
 * @return Attribute rate
 */
int GetAttributeRate(int attribute_id, int rate);

/**
 * Gets the attribute damage multiplier/protection (A-E).
 *
 * @param attr Attribute to test
 * @param rate Attribute rate to get
 * @return Attribute rate
 */
int GetAttributeRate(const lcf::rpg::Attribute& attr, int rate);

/**
 * Modifies the effect by weapon attributes against the target.
 *
 * @param effect Base effect to adjust
 * @param weapon1 First weapon being used against target. If nullptr or type is not weapon type, is ignored.
 * @param weapon2 Second weapon being used against target. If nullptr or type is not weapon type, is ignored.
 * @param target Target to apply attributes against
 * @return modified effect
 */
int ApplyAttributeMultiplier(int effect, const lcf::rpg::Item* weapon1, const lcf::rpg::Item* weapon2, const Game_Battler& target);

/**
 * Modifies the effect by weapon attributes against the target.
 *
 * @param effect Base effect to adjust
 * @param skill Skill being used against target
 * @param target Target to apply attributes against
 * @return modified effect
 */
int ApplyAttributeMultiplier(int effect, const lcf::rpg::Skill& skill, const Game_Battler& target);

/**
 * Modifies the effect by weapon attributes against the target.
 *
 * @param effect Base effect to adjust
 * @param attribute_sets A Span of attribute bitsets to check for attributes to apply against the target.
 * @param target Target to apply attributes against
 * @return modified effect
 */
int ApplyAttributeMultiplier(int effect, Span<const lcf::DBBitArray*> attribute_sets, const Game_Battler& target);

} // namespace Attribute


#endif
