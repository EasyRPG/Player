/*
 * This file is part of EasyRPG Player.
 * ... (license header) ...
 * Based on DynRPG Particle Effects V1 by Kazesui. (MIT license)
 */

#ifndef EP_DYNRPG_PARTICLE_V1_H_
#define EP_DYNRPG_PARTICLE_V1_H_

#include "game_dynrpg.h"

namespace DynRpg {
	class ParticleV1 : public DynRpgPlugin {
	public:
		ParticleV1(Game_DynRpg& instance);
		~ParticleV1() override;

		bool Invoke(std::string_view func, dyn_arg_list args, bool& do_yield, Game_Interpreter* interpreter) override;
		void Update() override;
        void OnMapChange();
	};
}

#endif
