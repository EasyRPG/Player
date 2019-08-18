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
            // Move to different directory to prevent Save file collisions in IDBFS
            if (tmp.length > 1) {
                tmp[1] = tmp[1].toLowerCase();
                FS.mkdir(tmp[1]);
                FS.chdir(tmp[1]);
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

Module.arguments.push("easyrpg-player");
Module.arguments = Module.arguments.concat(parseargs());

// Use IDBFS for Save storage when the filesystem was not
// overwritten by a custom emscripten shell file
if (typeof(Module.EASYRPG_FS) === "undefined") {
    Module.EASYRPG_FS = IDBFS;
}

// Try resuming the audio playback because Chrome automutes it when there was
// no user interaction
function enableAudio() {
    var audio_start_timer = function() {
        setTimeout(function() {
            if (Module.SDL2 != undefined && Module.SDL2.audioContext != undefined) {
                if (Module.SDL2.audioContext.state == 'suspended') {
                    Module.SDL2.audioContext.resume();
                    audio_start_timer();
                }
            } else {
                audio_start_timer();
            };
        }, 3000);
    };

    audio_start_timer();
}
enableAudio();
