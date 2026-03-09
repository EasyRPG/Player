/*
 * This file is part of EasyRPG Player.
 * ... (license header) ...
 * Based on DynRPG Particle Effects by Kazesui. (MIT license)
 */

#ifndef EP_DYNRPG_PARTICLE_H_
#define EP_DYNRPG_PARTICLE_H_

#include "game_dynrpg.h"

namespace DynRpg {
	class Particle : public DynRpgPlugin {
	public:
		Particle(Game_DynRpg& instance); // <- Body removed, this is now a declaration
		~Particle() override;

		bool Invoke(std::string_view func, dyn_arg_list args, bool& do_yield, Game_Interpreter* interpreter) override;
		void Update() override;

		void OnMapChange();
	};
}

#endif
