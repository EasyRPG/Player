Module.EASYRPG_GAME = ""

function parseargs() {
    var tmp = [];
    var ret = [];
    var items = location.search.substr(1).split("&");

    // Store saves in subdirectory Save
    ret.push("--save-path");
    ret.push("Save");

    for (var index = 0; index < items.length; index++) {
        tmp = items[index].split("=");

        if (tmp[0] == "project-path" || tmp[0] == "save-path") {
            // Filter arguments that are set by us
            continue;
        }

        if (tmp[0] == "game") {
            // Filesystem is not ready when processing arguments, store path to game
            if (tmp.length > 1) {
                Module.EASYRPG_GAME = tmp[1].toLowerCase();
            }
        }
        ret.push("--" + tmp[0]);
        if (tmp.length > 1) {
            var arg = decodeURI(tmp[1]);
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

Module.arguments = ["easyrpg-player"];
Module.arguments = Module.arguments.concat(parseargs());
