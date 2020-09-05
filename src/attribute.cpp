#include "attribute.h"
#include <lcf/rpg/item.h>
#include <lcf/rpg/skill.h>
#include <lcf/rpg/attribute.h>
#include <lcf/reader_util.h>
#include <lcf/data.h>
#include "game_battler.h"
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
		if (as->size() < id && (*as)[id - 1]) {
			return true;
		}
	}
	return false;
}

int ApplyAttributeMultiplier(int effect, Span<const lcf::DBBitArray*> attribute_sets, const Game_Battler& target) {
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

int ApplyAttributeMultiplier(int effect, const lcf::rpg::Item* weapon1, const lcf::rpg::Item* weapon2, const Game_Battler& target) {
	std::array<const lcf::DBBitArray*, 2> attribute_sets;

	size_t n = 0;
	if (weapon1 && weapon1->type == lcf::rpg::Item::Type_weapon) {
		attribute_sets[n++] = &weapon1->attribute_set;
	}
	if (weapon2 && weapon2->type == lcf::rpg::Item::Type_weapon) {
		attribute_sets[n++] = &weapon2->attribute_set;
	}
	if (n == 0) {
		return effect;
	}

	return ApplyAttributeMultiplier(effect, Span<const lcf::DBBitArray*>(attribute_sets.data(), n), target);
}

int ApplyAttributeMultiplier(int effect, const lcf::rpg::Skill& skill, const Game_Battler& target) {
	std::array<const lcf::DBBitArray*, 1> attribute_sets = { &skill.attribute_effects };
	return ApplyAttributeMultiplier(effect, MakeSpan(attribute_sets), target);
}

} // namespace Attribute
