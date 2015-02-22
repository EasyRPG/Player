function parseargs() {
    var tmp = [];
    var ret = [];
    var items = location.search.substr(1).split("&");
    for (var index = 0; index < items.length; index++) {
        tmp = items[index].split("=");
        ret.push("--" + tmp[0]);
        if (tmp.length > 1) {
		    arg = decodeURI(tmp[1]);
			// split except if it's a string
			if (arg.length > 0) {
			    if (arg.slice(0) == "\"" && arg.slice(-1) == "\"") {
				    ret.push(arg.slice(1, -1));
				} else {
					var spl = arg.split(" ");
					ret = ret.concat(spl);
				}
			}
            
        }
    }
    return ret;
}

Module.arguments.push("easyrpg-player");
Module.arguments = Module.arguments.concat(parseargs());
