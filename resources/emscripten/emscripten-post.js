// Move to different directory to prevent Save file collisions in IDBFS
FS.mkdir("easyrpg");
FS.chdir("easyrpg");
if (Module.EASYRPG_GAME.length > 0) {
    FS.mkdir(Module.EASYRPG_GAME);
    FS.chdir(Module.EASYRPG_GAME);
}

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
