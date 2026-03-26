//
//  _lua_scripts.cpp
//  leasy
//
//  Created by Hüseyin ÖZTÜRK on 26/03/2026.
//

namespace leasy::lscripts {
	const char *set_require = R"(local old_require = require
function require(module_name)
		if not package.loaded[module_name] then
			print("[leasy.Server.Loader]: loading:", module_name, "not loaded")
		else
		 print("[leasy.Server.Loader]: loading:", module_name, "chached")
		end
		return old_require(module_name)
end)";
}
