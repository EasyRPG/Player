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

#ifndef EP_DB_H
#define EP_DB_H

#include <lcf/rpg/fwd.h>
#include "span.h"
#include "idutils.h"

#include <memory>
#include <cassert>

namespace db {

/** @return database actors */
Span<const lcf::rpg::Actor> Actors();

/** @return database skills */
Span<const lcf::rpg::Skill> Skills();

/** @return database items */
Span<const lcf::rpg::Item> Items();

/** @return database enemies */
Span<const lcf::rpg::Enemy> Enemies();

/** @return database troops */
Span<const lcf::rpg::Troop> Troops();

/** @return database terrains */
Span<const lcf::rpg::Terrain> Terrains();

/** @return database attributes */
Span<const lcf::rpg::Attribute> Attributes();

/** @return database states */
Span<const lcf::rpg::State> States();

/** @return database animations */
Span<const lcf::rpg::Animation> Animations();

/** @return database chipsets */
Span<const lcf::rpg::Chipset> Chipsets();

/** @return database commonevents */
Span<const lcf::rpg::CommonEvent> Commonevents();

/** @return database battlecommands */
const lcf::rpg::BattleCommands& Battlecommands();

/** @return database classes */
Span<const lcf::rpg::Class> Classes();

/** @return database battleranimations */
Span<const lcf::rpg::BattlerAnimation> Battleranimations();

/** @return database terms */
const lcf::rpg::Terms& Terms();

/** @return database system */
const lcf::rpg::System& System();

/** @return database switches */
Span<const lcf::rpg::Switch> Switches();

/** @return database variables */
Span<const lcf::rpg::Variable> Variables();

/** @return LDB database */
const lcf::rpg::Database& Database();

/** @return LMT treemap */
const lcf::rpg::TreeMap& Treemap();

/** Load a new game */
void LoadGame(const lcf::rpg::Database* db, const lcf::rpg::TreeMap* treemap);

/** Clear all state */
void Clear();

namespace _impl {
struct Db {
	const lcf::rpg::Database* database = nullptr;
	const lcf::rpg::TreeMap* treemap = nullptr;

	Span<const lcf::rpg::Actor> actors;
	Span<const lcf::rpg::Skill> skills;
	Span<const lcf::rpg::Item> items;
	Span<const lcf::rpg::Enemy> enemies;
	Span<const lcf::rpg::Troop> troops;
	Span<const lcf::rpg::Terrain> terrains;
	Span<const lcf::rpg::Attribute> attributes;
	Span<const lcf::rpg::State> states;
	Span<const lcf::rpg::Animation> animations;
	Span<const lcf::rpg::Chipset> chipsets;
	Span<const lcf::rpg::CommonEvent> commonevents;
	const lcf::rpg::BattleCommands* battlecommands = nullptr;
	Span<const lcf::rpg::Class> classes;
	Span<const lcf::rpg::BattlerAnimation> battleranimations;
	const lcf::rpg::Terms* terms = nullptr;
	const lcf::rpg::System* system = nullptr;
	Span<const lcf::rpg::Switch> switches;
	Span<const lcf::rpg::Variable> variables;
};

extern Db db;
} // impl

inline Span<const lcf::rpg::Actor> Actors() { return _impl::db.actors; }
inline Span<const lcf::rpg::Skill> Skills() { return _impl::db.skills; }
inline Span<const lcf::rpg::Item> Items() { return _impl::db.items; }
inline Span<const lcf::rpg::Enemy> Enemies() { return _impl::db.enemies; }
inline Span<const lcf::rpg::Troop> Troops() { return _impl::db.troops; }
inline Span<const lcf::rpg::Terrain> Terrains() { return _impl::db.terrains; }
inline Span<const lcf::rpg::Attribute> Attributes() { return _impl::db.attributes; }
inline Span<const lcf::rpg::State> States() { return _impl::db.states; }
inline Span<const lcf::rpg::Animation> Animations() { return _impl::db.animations; }
inline Span<const lcf::rpg::Chipset> Chipsets() { return _impl::db.chipsets; }
inline Span<const lcf::rpg::CommonEvent> Commonevents() { return _impl::db.commonevents; }
inline const lcf::rpg::BattleCommands& Battlecommands() { return *_impl::db.battlecommands; }
inline Span<const lcf::rpg::Class> Classes() { return _impl::db.classes; }
inline Span<const lcf::rpg::BattlerAnimation> Battleranimations() { return _impl::db.battleranimations; }
inline const lcf::rpg::Terms& Terms() { return *_impl::db.terms; }
inline const lcf::rpg::System& System() { return *_impl::db.system; }
inline Span<const lcf::rpg::Switch> Switches() { return _impl::db.switches; }
inline Span<const lcf::rpg::Variable> Variables() { return _impl::db.variables; }

inline const lcf::rpg::Database& Database() { return *_impl::db.database; }
inline const lcf::rpg::TreeMap& Treemap() { return *_impl::db.treemap; }

} // namespace db
#endif
