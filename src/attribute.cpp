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
#include "attribute.h"
#include <lcf/rpg/item.h>
#include <lcf/rpg/skill.h>
#include <lcf/rpg/attribute.h>
#include <lcf/reader_util.h>
#include <lcf/data.h>
#include "game_battler.h"
#include "game_actor.h"
#include "game_enemy.h"
#include "output.h"
#include "player.h"
#include <climits>

namespace Attribute {

int GetAttributeRateModifier(int attribute_id, int rate) {
	const auto* attribute = lcf::ReaderUtil::GetElement(lcf::Data::attributes, attribute_id);

	if (!attribute) {
		Output::Warning("GetAttributeRate: Invalid attribute ID {}", attribute_id);
		return 0;
	}

	return GetAttributeRateModifier(*attribute, rate);
}

int GetAttributeRateModifier(const lcf::rpg::Attribute& attr, int rate) {
	switch (rate) {
	case 0:
		return attr.a_rate;
	case 1:
		return attr.b_rate;
	case 2:
		return attr.c_rate;
	case 3:
		return attr.d_rate;
	case 4:
		return attr.e_rate;
	default:
		break;
	}

	return 0;
}

static bool HasAttribute(Span<const lcf::DBBitArray*> attribute_sets, int id) {
	for (auto* as: attribute_sets) {
		const auto idx = id - 1;
		if (idx < static_cast<int>(as->size()) && (*as)[idx]) {
			return true;
		}
	}
	return false;
}

int ApplyAttributeMultiplier(int effect, const Game_Battler& target, Span<const lcf::DBBitArray*> attribute_sets) {
	int physical = INT_MIN;
	int magical = INT_MIN;

	int n = 0;
	for (auto* as: attribute_sets) {
		n = std::max(static_cast<int>(as->size()), n);
	}

	for (int i = 0; i < n; ++i) {
		const auto id = i + 1;

		if (!HasAttribute(attribute_sets, id)) {
			continue;
		}

		const auto* attr = lcf::ReaderUtil::GetElement(lcf::Data::attributes, id);
		if (!attr) {
			Output::Warning("ApplyAttributeMultipler: Invalid attribute ID {}", id);
			break;
		}

		const auto rate = target.GetAttributeRate(id);
		const auto mod = GetAttributeRateModifier(id, rate);
		if (attr->type == lcf::rpg::Attribute::Type_physical) {
			physical = std::max(physical, mod);
		} else {
			magical = std::max(magical, mod);
		}
	}

	// Negative attributes not supported in 2k.
	auto limit = Player::IsRPG2k() ? -1 : INT_MIN;

	if (physical > limit && magical > limit) {
		if (physical >= 0 && magical >= 0) {
			effect = magical * (physical * effect / 100) / 100;
		} else {
			effect = effect * std::max(physical, magical) / 100;
		}
	} else if (physical > limit) {
		effect = physical * effect / 100;
	} else if (magical > limit) {
		effect = magical * effect / 100;
	}
	return effect;
}

int ApplyAttributeNormalAttackMultiplier(int effect, const Game_Battler& source_battler, const Game_Battler& target, Game_Battler::Weapon weapon) {
	if (source_battler.GetType() != Game_Battler::Type_Ally && source_battler.GetType() != Game_Battler::Type_Enemy) {
		return effect;
	}

	std::array<const lcf::DBBitArray*, 2> attribute_sets = {{}};

	size_t n = 0;
	auto add = [&](int i) {
		if (source_battler.GetType() == Game_Battler::Type_Ally) {
			auto& source = static_cast<const Game_Actor&>(source_battler);
			if (source.GetWeapon() == nullptr && source.Get2ndWeapon() == nullptr) {
				lcf::rpg::Actor* allydata = lcf::ReaderUtil::GetElement(lcf::Data::actors, source.GetId());
				attribute_sets[n++] = &allydata->easyrpg_unarmed_attribute_set;
			} else {
				if (weapon == Game_Battler::Weapon(i + 1) || weapon == Game_Battler::WeaponAll) {
					auto* item = source.GetEquipment(i + 1);
					if (item && item->type == lcf::rpg::Item::Type_weapon) {
						attribute_sets[n++] = &item->attribute_set;
					}
				}
			}
		} else if (source_battler.GetType() == Game_Battler::Type_Enemy) {
			auto& source = static_cast<const Game_Enemy&>(source_battler);
			lcf::rpg::Enemy* enemydata = lcf::ReaderUtil::GetElement(lcf::Data::enemies, source.GetId());
			attribute_sets[n++] = &enemydata->easyrpg_attribute_set;
		}
	};

	for (int i = 0; i < static_cast<int>(attribute_sets.size()); ++i) {
		add(i);
	}

	return ApplyAttributeMultiplier(effect, target, Span<const lcf::DBBitArray*>(attribute_sets.data(), n));
}

int ApplyAttributeSkillMultiplier(int effect, const Game_Battler& target, const lcf::rpg::Skill& skill) {
	auto attribute_sets = Utils::MakeArray(&skill.attribute_effects);
	return ApplyAttributeMultiplier(effect, target, MakeSpan(attribute_sets));
}

} // namespace Attribute
