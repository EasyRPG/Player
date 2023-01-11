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

#include <cstdio>
#include <string>
#include <vector>
#include <psp2/io/stat.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/appmgr.h>
#include "player.h"

int _newlib_heap_size_user = 330 * 1024 * 1024;

int main(int argc, char* argv[]) {
	std::vector<std::string> args(argv, argv + argc);

	// Check if app is invoked with an externalized game path
	char boot_params[1024] = {};
	sceAppMgrGetAppParam(boot_params);
	std::string bp(boot_params);
	if (!bp.empty()) {
		// search for the start of params
		size_t start = bp.find("psgm:play");
		// ignore our titleid
		start = bp.find('&', start) + 1;
		// strip uninteresting stuff
		bp.assign(bp, start);

		if (!bp.empty()) {
			// based on Utils::Tokenize()
			std::string cur = "--";
			for (auto &c : bp) {
				// end of option
				if (c == '&') {
					args.push_back(cur);
					// next param starts
					cur = "--";
				}
				// end of argument
				else if (c == '=') {
					args.push_back(cur);
					// option starts
					cur.clear();
				}
				else
					cur.push_back(c);
			}
			// add last param/option
			args.push_back(cur);
		}
	} else {
		std::string psp2_dir = "ux0:/data/easyrpg-player";

		// Check if app0 filesystem contains the title id reference file
		FILE* f = fopen("app0:/titleid.txt", "r");
		if (f != NULL) {
			char title_id[10];
			std::string save_dir = "ux0:/data/";
			psp2_dir = "app0:/";

			fread(title_id, 1, 9, f);
			title_id[9] = 0;
			fclose(f);
			save_dir += title_id;

			// Create dirs if not existing
			sceIoMkdir(save_dir.c_str(), 0777);
			args.push_back("--save-path");
			args.push_back(save_dir);
		}

		sceIoMkdir(psp2_dir.c_str(), 0777);
		args.push_back("--project-path");
		args.push_back(psp2_dir);
	}

	// Run Player
	Player::Init(std::move(args));
	Player::Run();

	// Close
	sceKernelExitProcess(EXIT_SUCCESS);
	return EXIT_SUCCESS;
}
