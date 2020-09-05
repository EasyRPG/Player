#include "attribute.h"
#include <lcf/rpg/item.h>
#include <lcf/rpg/skill.h>
#include <lcf/rpg/attribute.h>
#include <lcf/reader_util.h>
#include <lcf/data.h>
#include "game_battler.h"
#include "game_actor.h"
#include "output.h"
#include "player.h"
#include <climits>

namespace Attribute {

int GetAttributeRate(int attribute_id, int rate) {
	const auto* attribute = lcf::ReaderUtil::GetElement(lcf::Data::attributes, attribute_id);

	if (!attribute) {
		Output::Warning("GetAttributeRate: Invalid attribute ID {}", attribute_id);
		return 0;
	}

	return GetAttributeRate(*attribute, rate);
}

int GetAttributeRate(const lcf::rpg::Attribute& attr, int rate) {
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
		if (static_cast<int>(as->size()) < id && (*as)[id - 1]) {
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

		const auto m = target.GetAttributeModifier(id);
		if (attr->type == lcf::rpg::Attribute::Type_physical) {
			physical = std::max(physical, m);
		} else {
			magical = std::max(magical, m);
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

int ApplyAttributeNormalAttackMultiplier(int effect, const Game_Actor& source, const Game_Battler& target, int weapon) {
	std::array<const lcf::DBBitArray*, 2> attribute_sets;

	size_t n = 0;
	auto add = [&](int i) {
		if (weapon == i || weapon == Game_Battler::kWeaponAll) {
			auto* item = source.GetEquipment(i);
			if (item && item->type == lcf::rpg::Item::Type_weapon) {
				attribute_sets[n++] = &item->attribute_set;
			}
		}
	};

	for (int i = 0; i < static_cast<int>(attribute_sets.size()); ++i) {
		add(i);
	}

	return ApplyAttributeMultiplier(effect, target, Span<const lcf::DBBitArray*>(attribute_sets.data(), n));
}

int ApplyAttributeSkillMultiplier(int effect, const Game_Battler& target, const lcf::rpg::Skill& skill) {
	std::array<const lcf::DBBitArray*, 1> attribute_sets = { &skill.attribute_effects };
	return ApplyAttributeMultiplier(effect, target, MakeSpan(attribute_sets));
}

} // namespace Attribute
