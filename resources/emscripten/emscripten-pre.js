// Note: The `Module` context is already initialized as an
// empty object by emscripten even before the pre script
Module = Object.assign(Module, {
  preRun: [],
  postRun: [],

  print: (...args) => {
    console.log(...args);
  },

  printErr: (...args) => {
    console.error(...args);
  },

  canvas: (() => {
    const canvas = document.getElementById('canvas');

    // As a default initial behavior, pop up an alert when webgl context is lost
    // See http://www.khronos.org/registry/webgl/specs/latest/1.0/#5.15.2
    canvas.addEventListener('webglcontextlost', event => {
      alert('WebGL context lost. You will need to reload the page.');
      event.preventDefault();
    }, false);

    return canvas;
  })(),

  setStatus: text => {
    if (!Module.setStatus.last) Module.setStatus.last = {
      time: Date.now(),
      text: ''
    };

    if (text !== Module.setStatus.text) {
      document.getElementById('status').innerHTML = text;
    }
  },

  totalDependencies: 0,

  monitorRunDependencies: left => {
    Module.totalDependencies = Math.max(Module.totalDependencies, left);
    Module.setStatus(left ? `Preparing... (${Module.totalDependencies - left}/${Module.totalDependencies})` : 'Downloading game data...');
  }
});

/**
 * Parses the current location query to setup a specific game
 */
function parseArgs () {
  const items = window.location.search.substr(1).split("&");
  let result = [];

  // Store saves in subdirectory `Save`
  result.push("--save-path");
  result.push("Save");

  for (let i = 0; i < items.length; i++) {
    const tmp = items[i].split("=");

    if (tmp[0] === "project-path" || tmp[0] === "save-path") {
      // Filter arguments that are set by us
      continue;
    }

    // Filesystem is not ready when processing arguments, store path to game
    if (tmp[0] === "game" && tmp.length > 1) {
      Module.EASYRPG_GAME = Module.EASYRPG_GAME + tmp[1].toLowerCase();
    }

    result.push("--" + tmp[0]);

    if (tmp.length > 1) {
      const arg = decodeURI(tmp[1]);
      // Split except if it's a string
      if (arg.length > 0) {
        if (arg.startsWith('"') && arg.endsWith('"')) {
          result.push(arg.slice(1, -1));
        } else {
          result = [...result, ...arg.split(" ")];
        }
      }
    }
  }

  return result;
}

if (typeof Module.EASYRPG_GAME === "undefined") {
  Module.EASYRPG_GAME = "";
}

Module.setStatus('Downloading...');
Module.arguments = ["easyrpg-player", ...parseArgs()];

// Catch all errors occuring inside the window
window.addEventListener('error', () => {
  Module.setStatus('Exception thrown, see JavaScript console…');
  Module.setStatus = text => {
    if (text) Module.printErr(`[post-exception status] ${text}`);
  };
});
