#include "db.h"
#include <lcf/rpg/database.h>
#include <lcf/rpg/treemap.h>
#include <lcf/rpg/map.h>

namespace db {

namespace _impl {
Db db = {};
} // _impl

/** Load a new game */
void LoadGame(const lcf::rpg::Database* database, const lcf::rpg::TreeMap* treemap) {
	Clear();

	_impl::db.database = database;
	_impl::db.treemap = treemap;

	_impl::db.actors = database->actors;
	_impl::db.skills = database->skills;
	_impl::db.items = database->items;
	_impl::db.enemies = database->enemies;
	_impl::db.troops = database->troops;
	_impl::db.terrains = database->terrains;
	_impl::db.attributes = database->attributes;
	_impl::db.states = database->states;
	_impl::db.animations = database->animations;
	_impl::db.chipsets = database->chipsets;
	_impl::db.commonevents = database->commonevents;
	_impl::db.battlecommands = &database->battlecommands;
	_impl::db.classes = database->classes;
	_impl::db.battleranimations = database->battleranimations;
	_impl::db.terms = &database->terms;
	_impl::db.system = &database->system;
	_impl::db.switches = database->switches;
	_impl::db.variables = database->variables;
}

/** Clear all state */
void Clear() {
	_impl::db = {};
}

} // namespace db
